#!/bin/bash

# Set the path to development version of the data commissioning software:
export SW_DEV_BASE_DIR="/sps/nemo/scratch/mauger/sw"

# Load Yves' Linuxbrew installation:
if [ -f ${SW_DEV_BASE_DIR}/SuperNEMO/config/linuxbrew_pro.bash ]; then
   source ${SW_DEV_BASE_DIR}/SuperNEMO/config/linuxbrew_pro.bash
fi
# Load Francois's Bayeux library installation:
if [ -f ${SW_DEV_BASE_DIR}/SuperNEMO/config/bayeux.bash ]; then
   source ${SW_DEV_BASE_DIR}/SuperNEMO/config/bayeux.bash
fi
# Load Francois's SNFEE library installation:
if [ -f ${SW_DEV_BASE_DIR}/SuperNEMO/config/snfee.bash ]; then
   source ${SW_DEV_BASE_DIR}/SuperNEMO/config/snfee.bash
fi

snfee_setup
