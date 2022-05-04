# Commissioning

Repository to share SuperNEMO commissioning analysis programs

## ReadRED

Base of C++ program to read Raw Event Data files

Compilation and usage at CC/IN2P3:

```
# retrieve the repository (you may also consider to fork and clone your own repository)
git clone https://github.com/SuperNEMO-DBD/Commissioning.git supernemo-commissioning
cd supernemo-commissioning/ReadRED/

# load CC/IN2P3 environnement
source setup_ccin2p3.sh

# build the demo program
mkdir build
cd build/
cmake ../
make
cd ../

# run demo with a RED file
build/read_red -i $RED_PATH/snemo_run-696_red.data.gz

# create your own program
cp read_red.cxx my_red_analysis.cxx
cd build/
cmake ../   # do only once, to update the Makefile with new rule for my_red_analysis compilation
make
cd ../
```

## ReadRTD

Base of C++ program to read Raw Trigger Data files

Better to work on RED files, unless you know what you want to do with RTD.
