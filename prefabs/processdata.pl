#musthave action 

#setifnotgiven data = "-"

#include $chunk_read

#proc processdata
action: @action
#ifspec fields
outfile: stdout
