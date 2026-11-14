CC ?= cc
CFLAGS ?= -O3 -std=c11
SRC = src/BLDualStrategy.c src/BlockBlastSolver.c
VALUE = learning/whole_tray_value_safe
TEST = learning/test_whole_tray_value
$(VALUE): learning/whole_tray_value_safe.c learning/learn_combo_guarded.c $(SRC)
	$(CC) $(CFLAGS) learning/whole_tray_value_safe.c $(SRC) -lm -o $@
$(TEST): learning/test_whole_tray_value.c learning/whole_tray_value_safe.c $(SRC)
	$(CC) $(CFLAGS) learning/test_whole_tray_value.c $(SRC) -lm -o $@
verify: $(TEST)
	./$(TEST)
	python3 -m py_compile learning/train_whole_tray.py
collect: $(VALUE)
	./$(VALUE) collect learning/whole_tray_train.csv 48
train:
	python3 learning/train_whole_tray.py --data learning/whole_tray_train.csv --output learning/models/whole_tray_value.json
benchmark: $(VALUE)
	./$(VALUE) test learning/models/whole_tray_value.json 12 5000000
.PHONY: verify collect train benchmark
