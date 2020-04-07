=====================================
SNFEE examples: read RTD data
=====================================

This example illustrates how to read the SuperNEMO raw data files (RTD)
and extract informations from the *calorimeter hit records*.

Two example programs are provided:

* ``snfee-rtd-read-calo`` (simple):

  - reads a set of RTD files,
  - extracts calorimeter hit records,
  - optionally prints the data,
  - optionally displays the associated waveform.
    
* ``snfee-rtd-ana-calo`` (complex):

  - reads a set of RTD files,
  - extracts calorimeter hit records,
  - optionally prints the data,
  - optionally displays the associated waveform,
  - performs some special analysis and measurements on waveforms (baseline, peak search, charge, time),
  - builds histograms,
  - computes mean waveforms per channel,
  - saves results in output files.

The ``SNFrontEndElectronics_`` library must be installed and setup on your system.

.. _SNFrontEndElectronics: https://gitlab.in2p3.fr/SuperNEMO-DBD/SNFrontEndElectronics


#. Checks the SNFrontEndElectronics setup:

   .. code:: bash

      $ snfee_setup
      $ which snfee-query
      $ snfee-query --help
   ..

#. Build from the source directory:

   .. code:: bash

      $ mkdir _build.d/
      $ cd _build.d
      $ cmake \
	     -DCMAKE_INSTALL_PREFIX=$(pwd)/../_install.d \
	     -DSNFrontEndElectronics_DIR=$(snfee-query --cmakedir) \
	     ..
      $ make
      $ make install
   ..

#. Run the ``snfee-rtd-read-calo`` program:

   .. code:: bash

      $ cd ../_install.d
      $ ./snfee-rtd-read-calo --help
   ..

   .. code:: bash

      $ cd ../_install.d
      $ ./snfee-rtd-read-calo \
	     --logging "debug" \
	     --input-file "/data/event/snemo_data/RTD/snemo_run-104_rtd_part-0.xml.gz" \ 
	     --max-rtd 100 \
	     --print-data \
	     --calo-display

#. Run the ``snfee-rtd-ana-calo`` program:

   .. code:: bash

      $ cd ../_install.d
      $ ./snfee-rtd-ana-calo --help
   ..

   .. code:: bash

      $ cd ../_install.d
      $ ./snfee-rtd-ana-calo \
	     --logging "debug" \
	     --input-file "/data/event/snemo_data/RTD/snemo_run-104_rtd_part-0.xml.gz" \
	     --output-file-histograms "snemo_run-104_rtd_histos.root" \
	     --low-threshold \
	     --calo-waveform-measurements \
	     --calo-mean-waveforms \
	     --calo-waveform-fft \
	     --calo-display

.. end
   
