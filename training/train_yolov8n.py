#!/usr/bin/env python3
"""Train a USER-LABELED YOLOv8n Block Blast detector and export to Core ML.

No trained model or proprietary website model is provided. This script refuses
to run until real screenshots are labeled in the documented three-class format.
You must comply with Ultralytics licensing (AGPL-3.0 or Enterprise).
"""
import argparse
import shutil
from pathlib import Path

SUPPORTED = {".jpg", ".jpeg", ".png", ".webp"}
CLASSES = ("board", "occupied_cell", "tray_cell")


def inspect_split(root: Path, split: str):
    images = root / "images" / split
    labels = root / "labels" / split
    if not images.is_dir() or not labels.is_dir():
        raise ValueError(f"Missing required folders: {images} and {labels}")
    files = sorted(p for p in images.iterdir() if p.suffix.lower() in SUPPORTED)
    counts = [0, 0, 0]
    for file in files:
        annotation = labels / f"{file.stem}.txt"
        if not annotation.is_file():
            raise ValueError(f"No annotation for {file}")
        board_seen = False
        tray_seen = False
        for lineno, line in enumerate(annotation.read_text().splitlines(), 1):
            if not line.strip():
                continue
            values = line.split()
            if len(values) != 5:
                raise ValueError(f"{annotation}:{lineno}: need class cx cy w h")
            kind = int(values[0])
            nums = [float(x) for x in values[1:]]
            if kind not in (0, 1, 2):
                raise ValueError(f"{annotation}:{lineno}: class 0/1/2 only")
            cx, cy, w, h = nums
            if not all(0 <= x <= 1 for x in (cx, cy, w, h)) or not w or not h:
                raise ValueError(f"{annotation}:{lineno}: invalid box")
            if cx - w/2 < 0 or cy - h/2 < 0 or cx + w/2 > 1 or cy + h/2 > 1:
                raise ValueError(f"{annotation}:{lineno}: box outside image")
            counts[kind] += 1
            board_seen |= kind == 0
            tray_seen |= kind == 2
        if not board_seen or not tray_seen:
            raise ValueError(f"{annotation}: each screenshot must show a board and tray")
    return len(files), counts


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset", type=Path, default=Path("training/dataset"))
    parser.add_argument("--epochs", type=int, default=120)
    parser.add_argument("--batch", type=int, default=16)
    parser.add_argument("--device", default=None, help="Auto device, cpu, mps, or a GPU index")
    parser.add_argument("--output", type=Path, default=Path("training/exports"))
    args = parser.parse_args()
    root = args.dataset.expanduser().resolve()
    trainN, trainCounts = inspect_split(root, "train")
    valN, valCounts = inspect_split(root, "val")
    print(f"Images: train={trainN}, validation={valN}; class train={trainCounts}, validation={valCounts}")
    if trainN < 100 or valN < 25 or min(trainCounts) < 100 or min(valCounts) < 25:
        raise SystemExit("Need >=100 annotated training screenshots and >=25 validation screenshots "
                         "covering each class before attempting a real-world YOLO release. "
                         "Use additional colors/skins, sparse/full boards, and game sessions.")
    config = args.output.resolve() / "dataset.yaml"
    config.parent.mkdir(parents=True, exist_ok=True)
    config.write_text(f"path: '{root}'\ntrain: images/train\nval: images/val\nnames:\n"
                      "  0: board\n  1: occupied_cell\n  2: tray_cell\n")
    from ultralytics import YOLO
    model = YOLO("yolov8n.pt")
    result = model.train(data=str(config), epochs=args.epochs, imgsz=640,
                         batch=args.batch, project=str(args.output.resolve()),
                         name="blockblast_yolov8n", exist_ok=True,
                         **({"device": args.device} if args.device else {}))
    # Verify held-out performance before exporting and shipping.
    best = Path(result.save_dir) / "weights" / "best.pt"
    if not best.is_file():
        raise FileNotFoundError(f"Training did not create {best}")
    trained = YOLO(str(best))
    metrics = trained.val(data=str(config), imgsz=640)
    print("Validation mAP50-95:", metrics.box.map,
          "mAP50:", metrics.box.map50, "recall:", metrics.box.mr)
    print("WARNING: mAP is box detection only. Exact board/tray reconstruction "
          "must be evaluated on unseen real screenshots before iPhone release.")
    package = Path(trained.export(format="coreml", imgsz=640, nms=True)).resolve()
    destination=args.output.resolve() / "BlockBlastYOLOv8n.mlpackage"
    if package!=destination:
        if destination.exists():
            shutil.rmtree(destination) if destination.is_dir() else destination.unlink()
        shutil.copytree(package, destination) if package.is_dir() else shutil.copy(package, destination)
    print("Exported Core ML package:", destination)
    print("On macOS: xcrun coremlcompiler compile", destination, args.output.resolve())
    print("Then bundle BlockBlastYOLOv8n.mlmodelc alongside BlockBlastAI.dylib in the IPA.")


if __name__ == "__main__":
    main()
