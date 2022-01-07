import struct
import sys
print(sys.argv[0])
print(sys.argv[1]) #参数都是str
print(len(sys.argv))

if len(sys.argv) > 2:
    input_file = sys.argv[2]
else:
    input_file = "flash.rom" 
print("in  file name:",input_file)
output_file = input_file.replace(".rom",".bin")
print("out file name:",output_file)
print(">>>>>>>>>>>>>>>>>>Flash_rom To Bin<<<<<<<<<<<<<<<<<<<<<<<<")


#Read data
print(">>Start Read Data")
f= open(input_file,"r")
list_lines=f.readlines()
f.close()


#write data
a= int(sys.argv[1]) #str转换成integer
print(">>start write File From ",a)
print(list_lines[a])
fw = open(file=output_file,mode="wb")
for i in list_lines[a:]:
    fw.write(struct.pack('L',int(i,16)))
fw.close()


print("************************END***********************")