#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <snfee/snfee.h>
#include <snfee/io/multifile_data_reader.h>

#include <sncabling/om_id.h>
#include <sncabling/gg_cell_id.h>
#include <sncabling/label.h>

#include <snemo/datamodels/raw_event_data.h>
#include <snemo/datamodels/calo_digitized_hit.h>
#include <snemo/datamodels/tracker_digitized_hit.h>

#include "sndisplay-demonstrator.cc"

// color codes for tracker hits in sndisplay
const float anode_and_two_cathodes  = 1;
const float anode_and_one_cathode   = 0.85;
const float anode_and_no_cathode    = 0.7;
const float two_cathodes_only       = 0.5;
const float one_cathode_only        = 0.2;

int main (int argc, char *argv[])
{
  const char *red_path = getenv("RED_PATH");

  int run_number = -1;
  int event_number = -1;

  int tracker_commissioning_area = -1;

  std::string input_filename = "";

  for (int iarg=1; iarg<argc; ++iarg)
    {
      std::string arg (argv[iarg]);
      if (arg[0] == '-')
	{
	  if (arg=="-i" || arg=="--input")
	    input_filename = std::string(argv[++iarg]);

	  else if (arg=="-r" || arg=="--run")
	    run_number = atoi(argv[++iarg]);

	  else if (arg=="-e" || arg=="--event")
	    event_number = atoi(argv[++iarg]);

	  else if (arg=="-a" || arg=="--area")
	    {
	      tracker_commissioning_area = atoi(argv[++iarg]);

	      if ((tracker_commissioning_area < 0) || (tracker_commissioning_area >=8))
		{
		  std::cerr << "*** wrong tracker commissioning area ([0-7])" << std::endl;
		  tracker_commissioning_area = -1;
		}
	    }
	  else if (arg=="-h" || arg=="--help")
	    {
	      std::cout << std::endl;
	      std::cout << "Usage:   " << argv[0] << " [options]" << std::endl;
	      std::cout << std::endl;
	      std::cout << "Options:   -h / --help" << std::endl;
	      std::cout << "           -i / --input  RED_FILE" << std::endl;
	      std::cout << "           -r / --run    RUN_NUMBER" << std::endl;
	      std::cout << "           -r / --event  EVENT_NUMBER" << std::endl;
	      std::cout << std::endl;
	      return 0;
	    }

	  else
	    std::cerr << "*** unkown option " << arg << std::endl;
	}
    }

  if (event_number == -1)
    {
      std::cerr << "*** missing event_number (-e/--event EVENT_NUMBER)" << std::endl;
      return 1;
    }

  if (input_filename.empty())
    {
      if (run_number == -1)
	{
	  std::cerr << "*** missing run_number (-r/--run RUN_NUMBER)" << std::endl;
	  return 1;
	}
      
      char input_filename_buffer[128];
      snprintf(input_filename_buffer, sizeof(input_filename_buffer),
	       "%s/snemo_run-%d_red.data.gz", red_path, run_number);
      input_filename = std::string(input_filename_buffer);
    }

  snfee::initialize();

  /// Configuration for raw data reader
  snfee::io::multifile_data_reader::config_type reader_cfg;
  reader_cfg.filenames.push_back(input_filename);

  // Instantiate a reader
  std::cout << "Opening " << input_filename << " ..." << std::endl;
  snfee::io::multifile_data_reader red_source (reader_cfg);

  // Working RED object
  snemo::datamodel::raw_event_data red;
    
  // RED counter
  std::size_t red_counter = 0;
  bool event_found = false;

  std::cout << "Searching for event " << event_number << " ..." << std::endl;

  while (red_source.has_record_tag())
    {
      // Check the serialization tag of the next record:
      DT_THROW_IF(!red_source.record_tag_is(snemo::datamodel::raw_event_data::SERIAL_TAG),
                  std::logic_error, "Unexpected record tag '" << red_source.get_record_tag() << "'!");

      // Load the next RED object:
      red_source.load(red);
      red_counter++;

      // Event number
      int32_t red_event_id = red.get_event_id();

      if (event_number != red_event_id)
	continue;

      event_found = true;

      // Container of merged TriggerID(s) by event builder
      const std::set<int32_t> & red_trigger_ids = red.get_origin_trigger_ids();

      // Digitized calo hits
      const std::vector<snemo::datamodel::calo_digitized_hit> red_calo_hits = red.get_calo_hits();

      // Digitized tracker hits
      const std::vector<snemo::datamodel::tracker_digitized_hit> red_tracker_hits = red.get_tracker_hits();

      // // Print RED infos
      // std::cout << "Event #" << red_event_id << " contains "
      // 		<< red_trigger_ids.size() << " TriggerID(s) with "
      // 		<< red_calo_hits.size() << " calo hit(s) and "
      // 		<< red_tracker_hits.size() << " tracker hit(s)"
      // 		<< std::endl;

      sndisplay::demonstrator *demonstrator_display = new sndisplay::demonstrator ("Demonstrator");
      demonstrator_display->setrange(0, 1);

      printf("\n");
      printf("=> %zd CALO HIT(s) :\n", red_tracker_hits.size());

      // Scan calo hits
      for (const snemo::datamodel::calo_digitized_hit & red_calo_hit : red_calo_hits)
	{
	  const snemo::datamodel::timestamp & reference_time = red_calo_hit.get_reference_time();
	  int64_t calo_tdc = reference_time.get_ticks(); // >>> 1 calo TDC tick = 6.25E-9 sec

	  const double calo_adc2mv = 2500./4096.;
	  const float calo_amplitude  = red_calo_hit.get_fwmeas_peak_amplitude() * calo_adc2mv / 8.0;

	  const sncabling::om_id om_id = red_calo_hit.get_om_id();
	  int om_side, om_wall, om_column, om_row, om_num;

	  if (om_id.is_main())
	    {
	      om_side   = om_id.get_side();
	      om_column = om_id.get_column();
	      om_row    = om_id.get_row();
	      om_num = om_side*20*13 + om_column*13 + om_row;

	      printf("M:%d.%02d.%02d  (OM%4d)   TDC = %12ld   Amplitude = %5.1f mV   ",
		     om_side, om_column, om_row, om_num, calo_tdc, -calo_amplitude);

	      printf("%s %s\n",
		     red_calo_hit.is_high_threshold() ? "[HT]" : "    ",
		     red_calo_hit.is_low_threshold_only() ? "[LT]" : "");
	    }

	  else if (om_id.is_xwall())
	    {
	      om_side   = om_id.get_side();
	      om_wall   = om_id.get_wall();
	      om_column = om_id.get_column();
	      om_row    = om_id.get_row();
	      om_num = 520 + om_side*64 +  om_wall*32 + om_column*16 + om_row;

	      printf("X:%d.%d.%d.%02d (OM%4d)   TDC = %12ld   Amplitude = %5.1f mV  ",
		     om_side, om_wall, om_column, om_row, om_num, calo_tdc, -calo_amplitude);

	      printf("%s %s\n",
		     red_calo_hit.is_high_threshold() ? "[HT]" : "    ",
		     red_calo_hit.is_low_threshold_only() ? "[LT]" : "");
	    }

	  else if (om_id.is_gveto())
	    {
	      om_side = om_id.get_side();
	      om_wall = om_id.get_wall();
	      om_column = om_id.get_column();
	      om_num = 520 + 128 + om_side*32 + om_wall*16 + om_column;

	      printf("G:%d.%d.%02d   (OM%4d)   TDC = %12ld   Ampl = %5.1f mV  ",
		     om_side, om_wall, om_column, om_num, calo_tdc, -calo_amplitude);

	      printf("%s %s\n",
		     red_calo_hit.is_high_threshold() ? "[HT]" : "    ",
		     red_calo_hit.is_low_threshold_only() ? "[LT]" : "");
	    }

	  if (red_calo_hit.is_high_threshold() || red_calo_hit.is_low_threshold_only())
	    demonstrator_display->setomcontent(om_num, 1);
	}

      printf("\n");
      printf("=> %zd TRACKER HIT(s) :\n", red_tracker_hits.size());

      // Scan tracker hits
      for (const snemo::datamodel::tracker_digitized_hit & red_tracker_hit : red_tracker_hits)
	{
	  const sncabling::gg_cell_id gg_id = red_tracker_hit.get_cell_id();

	  int cell_side  = gg_id.get_side();
	  int cell_row   = gg_id.get_row();
	  int cell_layer = gg_id.get_layer();
	  int cell_num = 113*9*cell_side + 9*cell_row + cell_layer;

	  bool has_anode = false;
	  bool has_bottom_cathode = false;
	  bool has_top_cathode = false;

	  // GG timestamps
	  const std::vector<snemo::datamodel::tracker_digitized_hit::gg_times> & gg_timestamps_v = red_tracker_hit.get_times();

	  if (gg_timestamps_v.size() == 0)
	    continue;

	  bool first_timestamps = true;

	  for (const snemo::datamodel::tracker_digitized_hit::gg_times & gg_timestamps : red_tracker_hit.get_times())
	    {
	      if (first_timestamps)
		{
		  printf("GG:%d.%03d.%1d", cell_side, cell_row, cell_layer);
		  first_timestamps = false;}
	      else
		printf("          ");

	      for (int r=0; r<5; r++)
		{
		  const snemo::datamodel::timestamp anode_timestamp = gg_timestamps.get_anode_time(r);
		  const int64_t anode_tdc = anode_timestamp.get_ticks();

		  if (anode_tdc != snfee::data::INVALID_TICKS)
		    {
		      printf("     R%d = %12lu", r, anode_tdc);
		      if (r == 0) has_anode = true;
		    }
		  else printf("                      ");
		}

	      {
		const snemo::datamodel::timestamp bottom_cathode_timestamp = gg_timestamps.get_bottom_cathode_time();
		const int64_t bottom_cathode_tdc = bottom_cathode_timestamp.get_ticks();

		if (bottom_cathode_tdc != snfee::data::INVALID_TICKS)
		  {
		    printf("     R5 = %12lu", bottom_cathode_tdc);
		    has_bottom_cathode = true;
		  }
		else printf("                      ");
	      }

	      {
		const snemo::datamodel::timestamp top_cathode_timestamp = gg_timestamps.get_top_cathode_time();
		const int64_t top_cathode_tdc = top_cathode_timestamp.get_ticks();

		if (top_cathode_tdc != snfee::data::INVALID_TICKS)
		  {
		    printf("     R6 = %12lu", top_cathode_tdc);
		    has_top_cathode = true;
		  }
		else printf("                      ");
	      }

	      printf("\n");

	      if (has_anode)
		{
		  if (has_bottom_cathode && has_top_cathode)
		    demonstrator_display->setggcontent(cell_num, anode_and_two_cathodes);
		  else if (has_bottom_cathode || has_top_cathode)
		    demonstrator_display->setggcontent(cell_num, anode_and_one_cathode);
		  else 
		    demonstrator_display->setggcontent(cell_num, anode_and_no_cathode);
		}
	      else
		{
		  if (has_bottom_cathode && has_top_cathode)
		    demonstrator_display->setggcontent(cell_num, two_cathodes_only);
		  else if (has_bottom_cathode || has_top_cathode)
		    demonstrator_display->setggcontent(cell_num, one_cathode_only);
		}

	    } // for (gg_timestamps)

	} // for (red_tracker_hit)

      printf("\n");

      std::string title = Form("RUN %d // TRIGGER ", run_number);
      
      bool first_trigger_id = true;

      for (const int32_t trigger_id : red_trigger_ids)
	{
	  if (first_trigger_id)
	    first_trigger_id = false;
	  else
	    title += "+";
	  
	  title += Form("%d", trigger_id);
	}

      demonstrator_display->settitle(title.c_str());
      demonstrator_display->draw_top();

      if (tracker_commissioning_area != -1)
	{
	  // put in gray color unused cells
	  int first_row = 14*tracker_commissioning_area;

	  if (tracker_commissioning_area >= 4)
	    first_row++;

	  int last_row = first_row + 14;

	  for (int cell_side=0; cell_side<2; ++cell_side)
	    for (int cell_row=0; cell_row<113; ++cell_row)
	      {
		if ((cell_row >= first_row) && (cell_row < last_row))
		  continue;

		for (int cell_layer=0; cell_layer<9; ++cell_layer)
		  demonstrator_display->setggcolor(cell_side, cell_row, cell_layer, kGray+1);
	      }

	  demonstrator_display->update_canvas();
	}

      demonstrator_display->canvas->SaveAs(Form("run-%d_event-%d.png", run_number, event_number));

      break;

    } // (while red_source.has_record_tag())

  if (!event_found)
    std::cerr << "=> Event was not found ! (only " << red_counter <<  " RED in this file)" << std::endl;
    
  snfee::terminate();

  return 0;
}

