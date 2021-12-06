#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <sncabling/om_id.h>
#include <sncabling/gg_cell_id.h>
#include <sncabling/label.h>

#include <snfee/snfee.h>
#include <snfee/io/multifile_data_reader.h>

#include <snfee/data/raw_event_data.h>
#include <snfee/data/calo_digitized_hit.h>
#include <snfee/data/tracker_digitized_hit.h>
#include <snfee/data/time.h>

int main (int argc, char *argv[])
{
  std::string input_filename = "";
  std::string output_filename = "output.root";

  for (int iarg=1; iarg<argc; ++iarg)
    {
      std::string arg (argv[iarg]);
      if (arg[0] == '-')
	{
	  if (arg=="-i" || arg=="--input")
	    input_filename = std::string(argv[++iarg]);

	  else if (arg=="-o" || arg=="--output")
	    output_filename = std::string(argv[++iarg]);

	  else
	    std::cerr << "*** unkown option " << arg << std::endl;
	}
    }

  if (input_filename.empty())
    {
      std::cerr << "*** missing input filename !" << std::endl;
      return 1;
    }

  snfee::initialize();

  /// Configuration for raw data reader
  snfee::io::multifile_data_reader::config_type reader_cfg;
  reader_cfg.filenames.push_back(input_filename);

  // Instantiate a reader
  snfee::io::multifile_data_reader red_source (reader_cfg);

  // Working RED object
  snfee::data::raw_event_data red;
    
  // RED counter
  std::size_t red_counter = 0;

  while (red_source.has_record_tag())
    {
      // Check the serialization tag of the next record:
      DT_THROW_IF(!red_source.record_tag_is(snfee::data::raw_event_data::SERIAL_TAG),
                  std::logic_error, "Unexpected record tag '" << red_source.get_record_tag() << "'!");

      // Load the next RED object:
      red_source.load(red);

      // Run number
      int32_t red_run_id   = red.get_run_id();

      // Event number
      int32_t red_event_id = red.get_event_id();

      // Reference time from trigger
      const snfee::data::timestamp & red_reference_time = red.get_reference_time();

      // Container of merged TriggerID(s) by event builder
      const std::set<int32_t> & red_trigger_ids = red.get_origin_trigger_ids();

      // Digitized calo hits
      const std::vector<snfee::data::calo_digitized_hit> red_calo_hits = red.get_calo_hits();

      // Digitized tracker hits
      const std::vector<snfee::data::tracker_digitized_hit> red_tracker_hits = red.get_tracker_hits();

      // Print RED infos
      std::cout << "Event #" << red_event_id << " contains "
		<< red_trigger_ids.size() << " TriggerID(s) with "
		<< red_calo_hits.size() << " calo hits and "
		<< red_tracker_hits.size() << " tracker hit(s)"
		<< std::endl;

      // Scan calo hits
      for (const snfee::data::calo_digitized_hit & red_calo_hit : red_calo_hits)
	{
	  // Origin of the hit in RTD file
	  const snfee::data::calo_digitized_hit::rtd_origin & origin = red_calo_hit.get_origin();
	  // origin.get_trigger_id()
	  // origin.get_hit_number()

	  // OM ID from SNCabling
	  const sncabling::om_id om_id = red_calo_hit.get_om_id();

	  // Reference time (TDC)
	  const snfee::data::timestamp & reference_time = red_calo_hit.get_reference_time();
	  int64_t calo_tdc = reference_time.get_ticks();

	  // Digitized waveform
	  const std::vector<int16_t> & waveform = red_calo_hit.get_waveform();

	  // // High/Low threshold flags
	  // red_calo_hit.is_high_threshold();
	  // red_calo_hit.is_low_threshold_only();

	  // // Wavecatcher firmware measurement
	  // int16_t baseline       = red_calo_hit.get_fwmeas_baseline();
	  // int16_t peak_amplitude = red_calo_hit.get_fwmeas_peak_amplitude();
	  // int16_t peak_cell      = red_calo_hit.get_fwmeas_peak_cell();
	  // int32_t charge         = red_calo_hit.get_fwmeas_charge();
	  // int32_t rising_cell    = red_calo_hit.get_fwmeas_rising_cell();
	  // int32_t falling_cell   = red_calo_hit.get_fwmeas_falling_cell()
	}

      // Scan tracker hits
      for (const snfee::data::tracker_digitized_hit & red_tracker_hit : red_tracker_hits)
	{
	  // Origin of the hit in RTD file
	  // [...]

	  // CELL ID from SNCabling
	  const sncabling::gg_cell_id gg_id = red_tracker_hit.get_cell_id();

	  // GG timestamps
	  const std::vector<snfee::data::tracker_digitized_hit::gg_times> & gg_timestamps_v = red_tracker_hit.get_times();

	  // Scan timestamps
	  if (gg_timestamps_v.size() == 1)
	    {
	      // Case without multiple hit in the same category
	      const snfee::data::tracker_digitized_hit::gg_times & gg_timestamps = gg_timestamps_v.front();

	      // ANODE timestamps
	      const snfee::data::timestamp anode_timestamp_r0 = gg_timestamps.get_anode_time(0);
	      const int64_t anode_tdc_r0 = anode_timestamp_r0.get_ticks();

	      const snfee::data::timestamp anode_timestamp_r1 = gg_timestamps.get_anode_time(1);
	      const int64_t anode_tdc_r1 = anode_timestamp_r1.get_ticks();

	      const snfee::data::timestamp anode_timestamp_r2 = gg_timestamps.get_anode_time(2);
	      const int64_t anode_tdc_r2 = anode_timestamp_r2.get_ticks();

	      const snfee::data::timestamp anode_timestamp_r3 = gg_timestamps.get_anode_time(3);
	      const int64_t anode_tdc_r3 = anode_timestamp_r3.get_ticks();

	      const snfee::data::timestamp anode_timestamp_r4 = gg_timestamps.get_anode_time(4);
	      const int64_t anode_tdc_r4 = anode_timestamp_r4.get_ticks();

	      // CATHODE timestamps
	      const snfee::data::timestamp bottom_cathode_timestamp = gg_timestamps.get_bottom_cathode_time();
	      const int64_t bottom_cathode_tdc = bottom_cathode_timestamp.get_ticks();

	      const snfee::data::timestamp top_cathode_timestamp = gg_timestamps.get_top_cathode_time();
	      const int64_t top_cathode_tdc = top_cathode_timestamp.get_ticks();
	    }
	}

      // Increment the counter
      red_counter++;

    } // (while red_source.has_record_tag())
 
  std::cout << "Total number of RED objects       : " << red_counter << std::endl;

  snfee::terminate();

  return 0;
}

