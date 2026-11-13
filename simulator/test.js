// Standalone Node regression tests for the exact engine used by simulator/index.html.
const fs=require('node:fs'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(require('node:path').join(__dirname,'index.html'),'utf8');
const a=html.indexOf('<script>'),b=html.indexOf('</script>',a);
if(a<0||b<0)throw Error('Simulator engine script missing');
const context={module:{exports:{}},exports:{}};
vm.runInNewContext(html.slice(a+8,b),context,{timeout:5000,filename:'simulator-engine.js'});
const E=context.module.exports;
let n=0;
function test(name,fn){fn();n++;console.log('PASS '+name);}
test('real screenshot example includes 12 filled squares',()=>{
 const g=new E.Game();assert.equal(g.board.reduce((a,b)=>a+b),12);
 assert.equal(g.pieces.map(p=>p.length).join(','),'6,3,6');
});
test('suggested moves are legal and clear simultaneously completed lines',()=>{
 const g=new E.Game(),ans=E.solve(g.board,g.pieces);assert.ok(ans);
 let b=g.board.slice(),used=0;
 for(const m of ans.moves){
   assert.equal(used&(1<<m.piece),0);
   assert.ok(E.canPlace(b,g.pieces[m.piece],m.row,m.col));
   b=E.apply(b,g.pieces[m.piece],m.row,m.col).board;
   used|=1<<m.piece;
 }
 const row=Array(64).fill(0);
 for(let x=0;x<7;x++)row[x]=1;
 for(let y=1;y<8;y++)row[y*8+7]=1;
 assert.equal(E.apply(row,[[0,0]],0,7).lines,2);
});
test('scanner requires three identical frames before recommending a move',()=>{
 const d=new E.ScanGuard(),o=new E.Game().snapshot();
 assert.equal(d.ingest(o),null);assert.equal(d.ingest(o),null);
 assert.ok(d.ingest(o));assert.equal(d.status,'active');
});
test('false board cells cannot overwrite prior confirmed game state',()=>{
 const d=new E.ScanGuard(),o=new E.Game().snapshot();
 for(let k=0;k<3;k++)d.ingest(o);
 for(let k=0;k<10;k++){
   const bad={...o,board:o.board.slice()};bad.board[0]=1;
   assert.equal(d.ingest(bad),null);assert.equal(d.accepted,null);
 }
 for(let k=0;k<3;k++)d.ingest(o);
 assert.equal(d.status,'active');
});
test('false tray pieces cannot overwrite prior confirmed game state',()=>{
 const d=new E.ScanGuard(),o=new E.Game().snapshot();
 for(let k=0;k<3;k++)d.ingest(o);
 for(let k=0;k<10;k++){
   const bad={...o,pieces:E.clonePieces(o.pieces)};
   bad.pieces[0]=[[0,0],[1,0]];
   assert.equal(d.ingest(bad),null);assert.equal(d.accepted,null);
 }
});
test('capture dropouts remove highlights and recover automatically',()=>{
 const d=new E.ScanGuard(),o=new E.Game().snapshot();
 for(let k=0;k<3;k++)d.ingest(o);
 d.fail('capture unavailable');assert.equal(d.accepted,null);
 for(let k=0;k<3;k++)d.ingest(o);
 assert.equal(d.status,'active');
});
test('valid scanner state remains active over 60 minutes',()=>{
 const d=new E.ScanGuard(),o=new E.Game().snapshot();
 for(let k=0;k<3600;k++)d.ingest(o);
 assert.equal(d.status,'active');
});
test('60 seeded simulated play sessions never suggest illegal placements',()=>{
 for(let seed=1;seed<=60;seed++){
   const g=new E.Game(seed);g.load(0);
   for(let turn=0;turn<8;turn++){
     const r=E.solve(g.board,g.pieces);if(!r)break;
     const m=r.moves[0];assert.ok(E.canPlace(g.board,g.pieces[m.piece],m.row,m.col));
     assert.ok(g.move(m.piece,m.row,m.col));
   }
 }
});
console.log('PASS '+n+' GitHub CI simulator suites');
