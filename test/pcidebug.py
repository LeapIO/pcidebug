import cmd
import readline
from ctypes import *

libpcidebug = CDLL("./libpcidebug.so")
libpcidebug.pcidebug_showbar.argtype = (c_int, c_int, c_uint64, c_uint64)
libpcidebug.pcidebug_rdbar8.argtype = (c_int, c_int, c_uint64)
libpcidebug.pcidebug_rdbar8.restype = c_uint8
libpcidebug.pcidebug_rdbar16.argtype = (c_int, c_int, c_uint64)
libpcidebug.pcidebug_rdbar16.restype = c_uint16
libpcidebug.pcidebug_rdbar32.argtype = (c_int, c_int, c_uint64)
libpcidebug.pcidebug_rdbar32.restype = c_uint32
libpcidebug.pcidebug_rdbar64.argtype = (c_int, c_int, c_uint64)
libpcidebug.pcidebug_rdbar64.restype = c_uint64
libpcidebug.pcidebug_wrbar8.argtype = (c_int, c_int, c_uint64, c_uint8)
libpcidebug.pcidebug_wrbar8.restype = c_uint8
libpcidebug.pcidebug_wrbar16.argtype = (c_int, c_int, c_uint64, c_uint16)
libpcidebug.pcidebug_wrbar16.restype = c_uint16
libpcidebug.pcidebug_wrbar32.argtype = (c_int, c_int, c_uint64, c_uint32)
libpcidebug.pcidebug_wrbar32.restype = c_uint32
libpcidebug.pcidebug_wrbar64.argtype = (c_int, c_int, c_uint64, c_uint64)
libpcidebug.pcidebug_wrbar64.restype = c_uint64
resultstr={True:"Success",False:"Failure"}

def parse_arg(args,cmd):
    id = int(args[0],10)
    offset = 0
    length = 24
    if "0x" in args[1]:
        offset = int(args[1],16)
    else:
        offset = int(args[1],10)
    val = 0
    if cmd == "write":
        if "0x" in args[2]:
            val = int(args[2],16)
        else:
            print("write value must begin with '0x'")
            raise Exception("write value error")
        return (id,offset,val)
    elif cmd == "read":
        return (id,offset)
    elif cmd == "show":
        if len(args)==3:
            length = int(args[2],10)
        return (id,offset,length)

class PCIdebugShell(cmd.Cmd):
    intro = 'Welcome to the PCIdebug shell. Type help or ? to list commannds.\n'
    prompt = 'PCIdebug$ '
    
    # ------ open/close device -----
    def preloop(self):
        self.fd = libpcidebug.pcidebug_open()
        if 0>self.fd:
            exit()

    def postloop(self):
        libpcidebug.pcidebug_close(self.fd)

    # ------ basic ioctl commands ----- 
    def do_show(self,arg):
        args = arg.split()
        if 2<=len(args) and len(args)<=3:
            (id, offset, length) = parse_arg(args,"show")
            libpcidebug.pcidebug_showbar(self.fd, c_int(id), c_uint64(offset), c_uint64(length))
        else:
            print("number of arguments error, it takes 2 arguments at least: barid offset length")

    def do_r8(self,arg):
        args = arg.split()
        if 2 == len(args):
            try:
                (id, offset) = parse_arg(args,"read")
                val = libpcidebug.pcidebug_rdbar8(self.fd, c_int(id), c_uint64(offset))
                print("BAR%d %#x = 0x%02x"%(id, offset, val))
            except:
                print("arguments error")
        else:
            print("number of arguments error, it takes 2 arguments")

    def do_r16(self,arg):
        args = arg.split()
        if 2 == len(args):
            try:
                (id, offset) = parse_arg(args,"read")
                val = libpcidebug.pcidebug_rdbar16(self.fd, c_int(id), c_uint64(offset))
                print("BAR%d %#x = 0x%04x"%(id, offset, val))
            except:
                print("arguments error")
        else:
            print("number of arguments error, it takes 2 arguments")

    def do_r32(self,arg):
        args = arg.split()
        if 2 == len(args):
            try:
                (id, offset) = parse_arg(args,"read")
                val = libpcidebug.pcidebug_rdbar32(self.fd, c_int(id), c_uint64(offset))
                print("BAR%d %#x = 0x%08x"%(id, offset, val))
            except:
                print("arguments error")
        else:
            print("number of arguments error, it takes 2 arguments")

    def do_r64(self,arg):
        args = arg.split()
        if 2 == len(args):
            try:
                (id, offset) = parse_arg(args,"read")
                val = libpcidebug.pcidebug_rdbar64(self.fd, c_int(id), c_uint64(offset))
                print("BAR%d %#x = 0x%016x"%(id, offset, val))
            except:
                print("arguments error")
        else:
            print("number of arguments error, it takes 2 arguments")

    def do_w8(self,arg):
        args = arg.split()
        if 3 == len(args):
            try:
                (id, offset, val) = parse_arg(args,"write")
                ret = libpcidebug.pcidebug_wrbar8(self.fd, c_int(id), c_uint64(offset), c_uint8(val))
                print("write %s, BAR%d %#x => 0x%02x"%(resultstr[ret==val], id, offset, ret))
            except:
                print("arguments error")
        else:
            print("number of arguments error, it takes 2 arguments")
    
    def do_w16(self,arg):
        args = arg.split()
        if 3 == len(args):
            try:
                (id, offset, val) = parse_arg(args,"write")
                ret = libpcidebug.pcidebug_wrbar16(self.fd, c_int(id), c_uint64(offset), c_uint16(val))
                print("write %s, BAR%d %#x => 0x%04x"%(resultstr[ret==val], id, offset, ret))
            except:
                print("arguments error")
        else:
            print("number of arguments error, it takes 2 arguments")
    
    def do_w32(self,arg):
        args = arg.split()
        if 3 == len(args):
            try:
                (id, offset, val) = parse_arg(args,"write")
                ret = libpcidebug.pcidebug_wrbar32(self.fd, c_int(id), c_uint64(offset), c_uint32(val))
                print("write %s, BAR%d %#x => 0x%08x"%(resultstr[ret==val], id, offset, ret))
            except:
                print("arguments error")
        else:
            print("number of arguments error, it takes 2 arguments")
    
    def do_w64(self,arg):
        args = arg.split()
        if 3 == len(args):
            try:
                (id, offset, val) = parse_arg(args,"write")
                ret = libpcidebug.pcidebug_wrbar64(self.fd, c_int(id), c_uint64(offset), c_uint64(val))
                print("write %s, BAR%d %#x => 0x%016x"%(resultstr[ret==val], id, offset, ret))
            except:
                print("arguments error")
        else:
            print("number of arguments error, it takes 2 arguments")


    def do_exit(self,arg):
        return True

        
if __name__ == '__main__':
    pcidebug = PCIdebugShell()
    try:
        pcidebug.cmdloop()
    except KeyboardInterrupt:
        print("")
        pcidebug.postloop()