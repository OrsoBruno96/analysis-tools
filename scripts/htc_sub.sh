#!/bin/sh 
# $1 - executable Runtime and Memory halved

cat > $1.submit <<EOF

+RequestRuntime=$2
RequestMemory = $3
executable = $1 
transfer_executable = True
universe            = vanilla
getenv              = True
Requirements        = OpSysAndVer == "SL6"
output              = $1.out
error               = $1.error
log                 = $1.log
queue
EOF

chmod u+x $1.submit
condor_submit $1.submit

