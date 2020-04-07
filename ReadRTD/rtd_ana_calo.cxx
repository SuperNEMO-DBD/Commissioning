// Standard library:
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>

// Third party:
// - Boost:
#include <boost/program_options.hpp>
// - Bayeux:
#include <bayeux/datatools/clhep_units.h>
#include <bayeux/datatools/temporary_files.h>
#include <bayeux/dpp/histogram_service.h>
#include <bayeux/mygsl/tabulated_sampling.h>
#include <bayeux/mygsl/tabulated_function.h>

// This project:
#include <snfee/snfee.h>
#include <snfee/io/multifile_data_reader.h>
#include <snfee/data/raw_trigger_data.h>
#include <snfee/data/calo_hit_record.h>
#include <snfee/data/channel_id.h>
#include <snfee/data/channel_id_selection.h>
#include <snfee/data/calo_waveform_drawer.h>
#include <snfee/data/calo_waveform_data.h>
#include <snfee/algo/calo_waveform_analysis.h>
#include <snfee/algo/calo_mean_waveform.h>
#include <snfee/algo/calo_waveform_tools.h>
#include <snfee/model/feb_constants.h>

// This example:
#include "calo_histogramming.h"
#include "calo_waveform_fft.h"

/// \brief Application configuration parameters
struct app_params_type
{
  /// Logging priority
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;
  
  /// Configuration for raw data reader: 
  snfee::io::multifile_data_reader::config_type reader_cfg;
  
  /// Maximum number of RTD records to be read
  uint32_t max_rtd = 0;
  
  /// Process only low-threshold calo hits  
  bool process_lt = false;
  
  /// Process only high-threshold calo hits
  bool process_ht = false;
  
  /// Channel ID selector
  snfee::data::channel_id_selection::config_type calo_channel_selector_cfg;
  
  /// Activation of the histogramming
  bool do_histogramming = false;
  
  /// Parameters for the histogram program
  snfee::calo::histogramming::config_type histogramming_cfg;
  
  /// Activation of the computation FFT on waveforms
  bool do_mean_waveforms = false;
  
  /// Parameters for the WF program
  snfee::algo::calo_mean_waveform_processor::config_type mean_waveform_cfg;
  
  /// Activation of measurements associated to a waveform (baseline, peak amplitude and position, charge, time...)
  bool do_waveform_measurements = false;
  
  /// Parameters for analysis algorithms
  std::string analysis_config_path;
  
  /// Activation of FFT on waveforms
  bool do_waveform_fft = false;
  
  /// Activation of visualization
  bool display = false;
  
};

int main(int argc_, char ** argv_)
{
  snfee::initialize();
  int error_code = EXIT_SUCCESS;
  try {

    // Configuration:
    app_params_type app_params;
    
    // Parse options:
    namespace po = boost::program_options;
    po::options_description opts("Allowed options");
    opts.add_options()
      ("help", "produce help message")

      ("logging,L",
       po::value<std::string>()->value_name("level"),
       "logging priority")

      ("input-file,i",
       po::value<std::vector<std::string>>(&app_params.reader_cfg.filenames)
       ->multitoken()
       ->value_name("path"),
       "add a RTD input filename")
  
      ("max-rtd,X",
       po::value<uint32_t>(&app_params.max_rtd)
       ->value_name("number"),
       "set the maximum number of processed RTD objects")
 
      ("low-threshold,l",
       po::value<bool>(&app_params.process_lt)->zero_tokens()->default_value(false),
       "Process only calo hits with low threshold (LT)")
 
      ("high-threshold,h",
       po::value<bool>(&app_params.process_ht)->zero_tokens()->default_value(false),
       "Process only calo hits with high threshold (HT)")
  
      ("calo-selected-crate,C",
       po::value<std::vector<int16_t>>(&app_params.calo_channel_selector_cfg.selected_crates)
       ->multitoken()
       ->value_name("number"),
       "add a crate number ([0-2]) to the calo hit channel selection")
  
      ("calo-selected-board,B",
       po::value<std::vector<int16_t>>(&app_params.calo_channel_selector_cfg.selected_boards)
       ->multitoken()
       ->value_name("number"),
       "add a board number ([0-19]) to the calo hit channel selection")
  
      ("calo-selected-channel,H",
       po::value<std::vector<int16_t>>(&app_params.calo_channel_selector_cfg.selected_channels)
       ->multitoken()
       ->value_name("number"),
       "add a board channel number ([0-15]) to the calo hit channel selection")
  
      ("calo-selected-channel-id,I",
       po::value<std::vector<std::string>>()
       ->multitoken()
       ->value_name("channel-id"),
       "add an explicit channel ID ('0.2.9') for calo hit channel selection ")
    
      ("calo-select-reverse,V",
       po::value<bool>(&app_params.calo_channel_selector_cfg.reverse)
       ->zero_tokens()
       ->default_value(false),
       "reverse the calo hit channel selection")
  
      ("output-file-histograms,o",
       po::value<std::string>(&app_params.histogramming_cfg.root_output_filename)
       ->value_name("path"),
       "set the ROOT histograms output filename")

      ("calo-analysis-config,A",
       po::value<std::string>(&app_params.analysis_config_path)
       ->value_name("path"),
       "set the calo waveforms analysis configuration path")
   
      ("calo-waveform-measurements,W",
       po::value<bool>(&app_params.do_waveform_measurements)
       ->zero_tokens()
       ->default_value(false),
       "do measurements on individual waveforms")
   
      ("calo-mean-waveforms,M",
       po::value<bool>(&app_params.do_mean_waveforms)
       ->zero_tokens()
       ->default_value(false),
       "compute mean waveforms per channel")
 
      ("calo-histogram-firmware,F",
       po::value<bool>(&app_params.histogramming_cfg.histo_from_firmware)
       ->zero_tokens()
       ->default_value(false),
       "use metadata from firmware for histogramming")
     
      ("calo-waveform-fft,T",
       po::value<bool>(&app_params.do_waveform_fft)
       ->zero_tokens()
       ->default_value(false),
       "compute waveform FFT")
     
      ("calo-display,D",
       po::value<bool>(&app_params.display)
       ->zero_tokens()
       ->default_value(false),
       "display the calo hit waveforms (and FFT is available)")

    ; // end of options description

    // Describe command line arguments :
    po::variables_map vm;
    po::store(po::command_line_parser(argc_, argv_)
              .options(opts)
              .run(), vm);
    po::notify(vm);

    // Use command line arguments :
    if (vm.count("help")) {
      std::cout << "snfee-rtd-ana-calo : "
                << "Read raw trigger data file (RTD) and extract calorimeter hit data"
                << std::endl << std::endl;
      std::cout << "Usage : " << std::endl << std::endl;
      std::cout << "  snfee-rtd-ana-calo [OPTIONS]" << std::endl << std::endl;  
      std::cout << opts << std::endl;
      std::cout << "Example : " << std::endl << std::endl;
      std::cout << " snfee-rtd-ana-calo \\\n";
      std::cout << "    --input-file \"snemo_run-8_rtd_part-0.data.gz\" \\\n";
      std::cout << "    --input-file \"snemo_run-8_rtd_part-1.data.gz\" \\\n";
      std::cout << "    --input-file \"snemo_run-8_rtd_part-2.data.gz\" \n";
      std::cout << "    --output-file-histograms \"snemo_run-8_rtd_calo_histos.root\" \n";
      std::cout << std::endl << std::endl;  
      return (-1);
    }

    // Use command line arguments :
    if (vm.count("logging")) {
      std::string logging_repr = vm["logging"].as<std::string>();
      app_params.logging = datatools::logger::get_priority(logging_repr);
      DT_THROW_IF(app_params.logging == datatools::logger::PRIO_UNDEFINED,
                  std::logic_error,
                  "Invalid logging priority '" << vm["logging"].as<std::string>() << "'!");
    }
    if (vm.count("output-file-histograms")) {
      app_params.do_histogramming = true;
    }

    // Use command line arguments :
    if (vm.count("calo-selected-channel-id")) {
      std::vector<std::string> ch_id_reprs = vm["calo-selected-channel-id"].as<std::vector<std::string>>();
      for (const auto & ch_id_repr : ch_id_reprs) {
        snfee::data::channel_id ch_id;
        DT_THROW_IF(!ch_id.from_string(ch_id_repr), std::logic_error,
                    "Invalid channel ID representation '" << ch_id_repr << "'!");
        app_params.calo_channel_selector_cfg.selected_ids.push_back(ch_id);
      }
    }

    // Raw calo hit measurement algorithms :
    std::unique_ptr<snfee::algo::calo_waveform_analysis> calo_analysis;
    if (app_params.do_waveform_measurements) {
      DT_LOG_DEBUG(app_params.logging, "Instantiating calo analysis...");
      snfee::algo::calo_waveform_analysis::config_type analysis_cfg; // Default configuration
      if (!app_params.analysis_config_path.empty()) {
        // Parse analysis parameters:
        analysis_cfg.parse(app_params.analysis_config_path);
      }
      calo_analysis.reset(new snfee::algo::calo_waveform_analysis(analysis_cfg));
      calo_analysis->initialize();
    } else {
      if (app_params.do_histogramming) {
        app_params.histogramming_cfg.histo_from_firmware = true;
      }
    }
  
    std::unique_ptr<snfee::algo::calo_waveform_fft> calo_fft;
    if (app_params.do_waveform_fft) {
      snfee::algo::calo_waveform_fft::config_type fft_cfg;
      calo_fft.reset(new snfee::algo::calo_waveform_fft(fft_cfg));
      calo_fft->initialize();
    }
 
    // Checks:
    DT_THROW_IF(app_params.reader_cfg.filenames.size() == 0,
                std::logic_error,
                "Missing input RTD filenames!");
   
    // Instantiate a reader:
    snfee::io::multifile_data_reader rtd_source(app_params.reader_cfg);

    // Working RTD object:
    snfee::data::raw_trigger_data rtd;
 
    // Histogramming:
    std::unique_ptr<snfee::calo::histogramming> calo_histogramming;
    if (app_params.do_histogramming) {
      calo_histogramming.reset(new snfee::calo::histogramming(app_params.histogramming_cfg));
      calo_histogramming->initialize();
    }
    
    // Mean waveform computing:
    std::unique_ptr<snfee::algo::calo_mean_waveform_processor> calo_mean_waveform;
    if (app_params.do_mean_waveforms) {
      calo_mean_waveform.reset(new snfee::algo::calo_mean_waveform_processor(app_params.mean_waveform_cfg));
      calo_mean_waveform->logging = datatools::logger::PRIO_NOTICE;
      calo_mean_waveform->initialize();
    }
    
    /// Waveform drawer:
    std::unique_ptr<snfee::data::calo_waveform_drawer> calo_drawer;
    if (app_params.display) {
      bool auto_range_time = true;
      bool auto_range_voltage = true;
      double time_min_ns = 0.0;
      double time_max_ns =
        snfee::model::feb_constants::SAMLONG_DEFAULT_TDC_LSB_NS
        * snfee::model::feb_constants::SAMLONG_MAX_NUMBER_OF_SAMPLES;
      double voltage_upper_mV = snfee::model::feb_constants::SAMLONG_ADC_MAX_VOLTAGE_MV / 5;
      double voltage_lower_mV = snfee::model::feb_constants::SAMLONG_ADC_MIN_VOLTAGE_MV;
      calo_drawer.reset(new snfee::data::calo_waveform_drawer(time_min_ns,
                                                              time_max_ns,
                                                              voltage_lower_mV,
                                                              voltage_upper_mV,
                                                              auto_range_time,
                                                              auto_range_voltage));
    }
        
    // Calorimeter hit channel ID selector:
    snfee::data::channel_id_selection calo_channel_selector(app_params.calo_channel_selector_cfg);

    // Loop on stored RTD objects:
    std::size_t rtd_counter = 0;
    std::size_t selection_counter = 0;
    while (rtd_source.has_record_tag()) {

      // Check the serialization tag of the next record:
      DT_THROW_IF(!rtd_source.record_tag_is(snfee::data::raw_trigger_data::SERIAL_TAG),
                  std::logic_error,
                  "Unexpected record tag '" << rtd_source.get_record_tag() << "'!");

      // Load the next RTD object:
      rtd_source.load(rtd);

      // Debug print:
      if (datatools::logger::is_debug(app_params.logging)) {
        boost::property_tree::ptree options;
        options.put("title", "Raw trigger data: ");
        options.put("indent", "[debug] ");
        rtd.print_tree(std::cerr, options);
      }

      // General informations:
      int32_t trigger_id = rtd.get_trigger_id();
      int32_t run_id     = rtd.get_run_id();

      // Loop on calo hit records in the RTD data object:
      for (const auto & p_calo_hit : rtd.get_calo_hits()) {
        
        // Dereference the stored shared pointer oin the calo hit record:
        const snfee::data::calo_hit_record & calo_hit = *p_calo_hit;
        
        if (datatools::logger::is_debug(app_params.logging)) {
          boost::property_tree::ptree options;
          options.put("title", "Loaded calorimeter raw hit data record: ");
          options.put("with_waveform_samples", true);
          calo_hit.print_tree(std::clog, options);
        }
       
        // General calo hit's data:
        int32_t  calo_trigger_id = calo_hit.get_trigger_id(); // Trigger unique ID associated to the calo hit
        uint64_t tdc             = calo_hit.get_tdc();        // TDC timestamp (48 bits)
        int32_t  crate_num       = calo_hit.get_crate_num();  // Crate number (0,1,2)
        int32_t  board_num       = calo_hit.get_board_num();  // Board number (0-19)
        int32_t  chip_num        = calo_hit.get_chip_num();   // Chip number (0-7)
        uint16_t fcr             = calo_hit.get_fcr();        // First cell read (TDC: 0-1023)
        
        // Waveform recording:
        bool     has_waveforms              = calo_hit.has_waveforms();                  // Default: true
        uint16_t waveform_start_sample      = calo_hit.get_waveform_start_sample();      // Default: 0 (TDC)
        uint16_t waveform_number_of_samples = calo_hit.get_waveform_number_of_samples(); // Default: 1024 (TDC)
        
        // Extract SAMLONG channels' data:
        for (int ichannel = 0; ichannel < snfee::model::feb_constants::SAMLONG_NUMBER_OF_CHANNELS; ichannel++) {
          
          const snfee::data::calo_hit_record::channel_data_record & ch_data = calo_hit.get_channel_data(ichannel);
          bool    ch_lt           = ch_data.is_lt();            // Low threshold flag
          bool    ch_ht           = ch_data.is_ht();            // High threshold flag
          bool    ch_underflow    = ch_data.is_underflow();     // Underflow flag
          bool    ch_overflow     = ch_data.is_overflow();      // Charge overflow flag
          int32_t ch_baseline     = ch_data.get_baseline();     // Computed baseline       (LSB: ADC unit/16)
          int32_t ch_peak         = ch_data.get_peak();         // Computed peak amplitude (LSB: ADC unit/8)
          int32_t ch_peak_cell    = ch_data.get_peak_cell();    // Computed peak position  (TDC: 0-1023)
          int32_t ch_charge       = ch_data.get_charge();       // Computed charge
          int32_t ch_rising_cell  = ch_data.get_rising_cell();  // Computed rising edge crossing (LSB: TDC unit/256)
          int32_t ch_falling_cell = ch_data.get_falling_cell(); // Computed falling edge crossing (LSB: TDC unit/256)

          // Compute a comprehensive readout Wavecatcher channel ID object (crate number+board number+channel number):
          snfee::data::channel_id ch_id(crate_num, // [0-2]
                                        board_num, // [0-9,11-20]
                                        snfee::model::feb_constants::SAMLONG_NUMBER_OF_CHANNELS * chip_num + ichannel); // [0-15]

          // Declare the waveform for this SAMLONG channel:
          std::vector<uint16_t> ch_waveform;

          // Check if the histograms should be filled for this calo hit:
          bool selected_channel = true;
          
          // Select hit to be histogrammed:
          if (app_params.process_lt and !ch_lt) {
            // Hit with LT flag:
            selected_channel = false;
          }
          if (app_params.process_ht and !ch_ht) {
            // Hit with HT flag:
            selected_channel = false;
          }
          if (!calo_channel_selector(ch_id)) {
            // Hit with allowed channel ID:
            selected_channel = false;
          }

          // Process selected channels:
          if (selected_channel) {

            // Constants:
            double tdc_to_ns = snfee::model::feb_constants::SAMLONG_DEFAULT_TDC_LSB_NS;
            double adc_to_mV = snfee::model::feb_constants::SAMLONG_ADC_VOLTAGE_LSB_MV;

            // Firmware metadata:
            double charge_nVs  = ch_charge * 1e-3 * adc_to_mV * tdc_to_ns;
            double peak_mV     = ch_peak * adc_to_mV / 8;
            double baseline_mV = ch_baseline * adc_to_mV / 16;
            
            // Waveform processing:
            if (has_waveforms) {
              
              // Extract ADC samples for the SAMLONG channel from the interleaved SAMLONG data:
              // Fill the waveform for this SAMLONG channel:
              ch_waveform.reserve(waveform_number_of_samples);
              for (int isample = 0; isample < waveform_number_of_samples; isample++) {
                int16_t adc = calo_hit.get_waveforms().get_adc(isample, ichannel); // 0-4095 (ADC)
                ch_waveform.push_back(adc); 
              }
              
              // Working waveform data structure (for dedicated measurements):
              snfee::data::calo_waveform_info waveform_info;

              // Waveform analysis and measurements:
              if (calo_analysis) {
                DT_LOG_DEBUG(app_params.logging, "Do calo waveform analysis...");
                // Processing waveforms:
                calo_analysis->init_from_raw_data(ch_id, ch_data, ch_waveform, waveform_info);
                calo_analysis->do_measurements(ch_id, waveform_info);
                if (datatools::logger::is_debug(app_params.logging)) {
                  waveform_info.print(std::cerr, "Waveform info : ", "[debug] ");
                }
              }

              if (calo_fft) {
                DT_LOG_DEBUG(app_params.logging, "Do calo waveform FFT...");
                std::vector<double> ft;
                std::vector<double> fwf;
                double frequency_step;
                calo_fft->transform(waveform_info.waveform, ft, fwf, frequency_step);
                if (app_params.display) {
                  std::string title = "Waveform FFT for calo channel [" + ch_id.to_string() + "]";
                  snfee::algo::calo_waveform_fft::display_waveform_fft(ft,
                                                                       frequency_step,
                                                                       title);
                }
              }
 
              // Mean waveform processing:
              if (calo_mean_waveform) {
                calo_mean_waveform->process_waveform(ch_id, waveform_info);
                if (app_params.histogramming_cfg.histo_from_firmware) {
                  // Using measurements from waveform analysis:
                  baseline_mV = waveform_info.baseline.baseline_mV;
                  peak_mV     = waveform_info.peak.amplitude_mV;
                  charge_nVs  = waveform_info.charge.charge_nVs;
                }
              }

              // Visualization of waveforms:
              if (calo_drawer) {
                if (waveform_info.waveform.size() == 0) {
                  // At least, draw the waveform shape:
                  int16_t adc_zero  = snfee::model::feb_constants::SAMLONG_ADC_ZERO;
                  snfee::algo::calo_waveform_analysis::populate_waveform(ch_waveform,
                                                                         waveform_info.waveform,
                                                                         tdc_to_ns,
                                                                         adc_zero,
                                                                         adc_to_mV);
                }
                bool display_this_one = true;
                if (display_this_one) {
                  calo_drawer->draw(waveform_info, "", ch_id.to_string());
                }
              }

            } // has_waveforms

            // Histogramming:
            if (calo_histogramming) {
              
              if (calo_histogramming->config.histo_charge) {
                calo_histogramming->fill(ch_id.to_string(), run_id, "charge", charge_nVs);
              }
              
              if (calo_histogramming->config.histo_peak) {
                calo_histogramming->fill(ch_id.to_string(), run_id, "peak", peak_mV);
              }
              
              if (calo_histogramming->config.histo_baseline) {
                calo_histogramming->fill(ch_id.to_string(), run_id, "baseline", baseline_mV);
              }
              
              if (calo_histogramming->config.histo_peak_charge) {
                calo_histogramming->fill(ch_id.to_string(), run_id, "peak_charge", peak_mV, charge_nVs);
              }
              
            }
            
            selection_counter++;

          } // end of selected channel

        } // end of channel loop in the current RTD record
        
      } // end of loop on calo hit records in the RTD data object

      rtd_counter++;
      if (rtd_counter % 500 == 0) {
        std::clog << "Number of read RTD objects: " << rtd_counter << std::endl;
      }
      if (app_params.max_rtd > 0 and rtd_counter == app_params.max_rtd) {
        break;
      }
      
    } // end of loop on stored RTD objects:
    
    // Report:
    std::clog << "Total number of RTD objects       : " << rtd_counter << std::endl;
    std::clog << "Total number of selected channels : " << selection_counter << std::endl;

    // Clean:
    if (calo_histogramming) {
      calo_histogramming->terminate();
      calo_histogramming.reset();
    }

    if (calo_mean_waveform) {
      calo_mean_waveform->terminate();
      calo_mean_waveform.reset();
    }
    
    if (calo_fft) {
      calo_fft->terminate();
      calo_fft.reset();
    }
 
    if (calo_analysis) {
      calo_analysis->terminate();
      calo_analysis.reset();
    }
 
    if (calo_drawer) {
      calo_drawer.reset();
    }
    
  } catch (std::exception & x) {
    std::cerr << "error: " << x.what() << std::endl;
    error_code = EXIT_FAILURE;
  } catch (...) {
    std::cerr << "error: " << "unexpected error!" << std::endl;
    error_code = EXIT_FAILURE;
  }
  snfee::terminate();
  return (error_code);
}
