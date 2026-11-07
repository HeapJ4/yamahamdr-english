"""PE32 resource reader and Win32 dialog/menu/string-table parser."""
import struct

def resources(path):
 b=path.read_bytes()
 if b[:2] != b'MZ': raise ValueError('Not a Windows executable: ' + str(path))
 pe=struct.unpack_from('<I',b,60)[0]; n=struct.unpack_from('<H',b,pe+6)[0]; sz=struct.unpack_from('<H',b,pe+20)[0]; opt=pe+24
 if b[pe:pe+4] != b'PE\0\0' or struct.unpack_from('<H',b,opt)[0] != 0x10b: raise ValueError('Expected a PE32 binary: ' + str(path))
 rva,size=struct.unpack_from('<II',b,opt+112)
 secs=[struct.unpack_from('<8sIIII',b,opt+sz+i*40) for i in range(n)]
 def off(r):
  for _,vs,va,rs,raw in secs:
   if va<=r<va+max(vs,rs): return raw+r-va
  raise ValueError(r)
 if not rva:return []
 base=off(rva); result=[]
 def walk(rel,keys):
  p=base+rel; a,c=struct.unpack_from('<HH',b,p+12)
  for i in range(a+c):
   name,child=struct.unpack_from('<II',b,p+16+i*8)
   if name&0x80000000:
    s=base+(name&0x7fffffff); length=struct.unpack_from('<H',b,s)[0]; name=b[s+2:s+2+length*2].decode('utf-16le')
   if child&0x80000000:walk(child&0x7fffffff,keys+[name])
   else:
    dr,ds,cp,_=struct.unpack_from('<IIII',b,base+child); start=off(dr)
    result.append((keys+[name],b[start:start+ds],start))
 walk(0,[]);return result
def text_fields(data,typ,alignments=None):
 fields=[]; p=0
 def u16():
  nonlocal p
  x=struct.unpack_from('<H',data,p)[0];p+=2;return x
 def field(label):
  nonlocal p
  start=p;x=u16()
  if x==65535:u16();return
  while x:x=u16()
  s=data[start:p-2].decode('utf-16le')
  if s:fields.append(dict(offset=start,end=p,text=s,label=label))
 def align():
  nonlocal p
  if alignments is not None:alignments.append((p,(p+3)&~3))
  p=(p+3)&~3
 if typ==6:
  for i in range(16):
   start=p;n=u16();s=data[p:p+n*2].decode('utf-16le');p+=n*2
   if s:fields.append(dict(offset=start,end=p,text=s,label=str(i),counted=True))
 elif typ==5:
  ext=data[:4]==b'\x01\x00\xff\xff'
  if ext:style=struct.unpack_from('<I',data,12)[0];count=struct.unpack_from('<H',data,16)[0];p=26
  else:style=struct.unpack_from('<I',data,0)[0];count=struct.unpack_from('<H',data,8)[0];p=18
  field('menu');field('class');field('caption')
  if style&64:p+=6 if ext else 2;field('font')
  for i in range(count):
   align();p+=24 if ext else 18;field('control-class');field('control-text');n=u16();p+=n
 elif typ==4:
  version,offset=struct.unpack_from('<HH',data,0);p=4+offset
  if version != 0: raise ValueError('Extended menus are not supported by this preview')
  if version==0:
   def menu():
    nonlocal p
    while True:
     flags=u16()
     if not flags&16:u16()
     field('menu')
     if flags&16:menu()
     if flags&128:return
   menu()
 return fields
