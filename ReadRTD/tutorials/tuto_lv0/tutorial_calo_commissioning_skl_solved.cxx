// Standard library:
#include <iostream>
#include <exception>
#include <cstdlib>


#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"


#include <snfee/snfee.h>
#include <snfee/io/multifile_data_reader.h>
#include <snfee/data/raw_trigger_data.h>


// This project:
#include <sncabling/sncabling.h>
#include <sncabling/om_id.h>
#include <sncabling/calo_hv_id.h>
#include <sncabling/calo_hv_cabling.h>
#include <sncabling/label.h>

#include <sncabling/service.h>
#include <sncabling/calo_signal_cabling.h>

bool debug = true;


void usage(){

  std::clog<<std::endl;
  std::clog<<"+--------------------------------------------------+"<<std::endl;
  std::clog<<"| SuperNEMO calorimeter commissioning tutorial lv0 |"<<std::endl;
  std::clog<<"+--------------------------------------------------+"<<std::endl;

  std::clog<<"How to : "<<std::endl;
  std::clog<<" "<<std::endl;
  std::clog<<std::endl;


}

// Main program

int main(int argc, char **argv)
{
  sncabling::initialize();  
  int error_code = EXIT_SUCCESS;
  
  try {
    
    if (argc > 1){
      usage();
    }


    sncabling::service snCabling;
    snCabling.initialize_simple();
    // Access to the calorimeter signal readout cabling map:
    const sncabling::calo_signal_cabling & caloSignalCabling
      = snCabling.get_calo_signal_cabling();
    


    TFile a_file("output.root","RECREATE");
    TH1F h1("amplitude", "a nice title", 400, -400.0, 0.0);

    
    /// Configuration for raw data reader
    snfee::io::multifile_data_reader::config_type reader_cfg;
    reader_cfg.filenames.push_back("/sps/nemo/snemo/snemo_data/raw_data/v1/RTD/run_341/output_data.d/snemo_run-341_rtd.data.gz");
    
    
    // Instantiate a reader:
    snfee::io::multifile_data_reader rtd_source(reader_cfg);
    
    // Working RTD object --> Raw Trigger Data 
    // 1 record per trigger composed by few CaloHit
    snfee::data::raw_trigger_data rtd; 
    
    std::size_t rtd_counter = 0;
    while (rtd_source.has_record_tag() ) {
      rtd_counter++;
      
      // Load the next RTD object:
      rtd_source.load(rtd);
      // General informations:
      int32_t trigger_id = rtd.get_trigger_id();
      int32_t run_id     = rtd.get_run_id();
      
      if(rtd_counter %10000 == 0 )std::clog<<"In Run : "<<run_id<<" Trigger # "<<trigger_id <<std::endl;
      
      std::size_t calo_counter = 0;
      // Loop on calo hit records in the RTD data object:
      for (const auto & p_calo_hit : rtd.get_calo_hits()) {
	// Dereference the stored shared pointer oin the calo hit record:
	const snfee::data::calo_hit_record & calo_hit = *p_calo_hit;
	calo_counter++;
	uint64_t tdc             = calo_hit.get_tdc();        // TDC timestamp (48 bits)
	int32_t  crate_num       = calo_hit.get_crate_num();  // Crate number (0,1,2)
	int32_t  board_num       = calo_hit.get_board_num();  // Board number (0-19)
	int32_t  chip_num        = calo_hit.get_chip_num();   // Chip number (0-7)

	
	if(rtd_counter < 100 ){
	  std::clog<<"   |-> tdc      : "<< tdc <<std::endl;
	  std::clog<<"   |-> calo data from CaloFEB : "<<crate_num<<"."<<board_num<<"."<<chip_num<<std::endl;
	}
	// Extract SAMLONG channels' data:
	// 2 channels per SAMLONG 
	for (int ichannel = 0; ichannel < snfee::model::feb_constants::SAMLONG_NUMBER_OF_CHANNELS; ichannel++) {
	  const snfee::data::calo_hit_record::channel_data_record & ch_data = calo_hit.get_channel_data(ichannel);
	  bool    ch_lt           = ch_data.is_lt();            // Low threshold flag
	  bool    ch_ht           = ch_data.is_ht();            // High threshold flag
	  int32_t ch_baseline     = ch_data.get_baseline();     // Computed baseline       (LSB: ADC unit/16)
	  int32_t ch_peak         = ch_data.get_peak();         // Computed peak amplitude (LSB: ADC unit/8)
	  int32_t ch_charge       = ch_data.get_charge();       // Computed charge


	  sncabling::calo_signal_id readout_id(sncabling::CALOSIGNAL_CHANNEL,
					       crate_num, 
					       board_num, 
					       snfee::model::feb_constants::SAMLONG_NUMBER_OF_CHANNELS * chip_num + ichannel);
	  if (caloSignalCabling.has_channel(readout_id)) {
	    const sncabling::om_id & calo_id = caloSignalCabling.get_om(readout_id);
	    std::clog <<"readout channel : "<<readout_id.to_label()<<std::endl;
	    std::clog <<"om id : "<<calo_id.to_label()<<std::endl;

	  }



	  h1.Fill(ch_peak);


	}//end of channels
      }//end of calohit
    }//end of file
    
    
    std::clog<<"Look after : "<<rtd_counter<<" entries"<<std::endl;
    a_file.Write();
    a_file.Close();

    

  } catch (std::exception & error) {
    std::cerr << "[error] " << error.what() << std::endl;
    error_code = EXIT_FAILURE;
  }
  sncabling::terminate();
  return error_code;
}
