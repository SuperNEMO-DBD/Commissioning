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

#include <snfee/data/raw_event_data.h>
#include <snfee/data/calo_digitized_hit.h>
#include <snfee/data/tracker_digitized_hit.h>

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

  int tracker_area = -1;
  int tracker_crate = -1;

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

	  else if (arg=="-a" || arg=="--tracker-area")
	    {
	      tracker_area = atoi(argv[++iarg]);

	      if ((tracker_area < 0) || (tracker_area >=8))
		{
		  std::cerr << "*** wrong tracker commissioning area ([0-7])" << std::endl;
		  tracker_area = -1;
		}
	    }

	  else if (arg=="-c" || arg=="--tracker-crate")
	    {
	      tracker_crate = atoi(argv[++iarg]);

	      if ((tracker_crate < 0) || (tracker_crate >=3))
		{
		  std::cerr << "*** wrong tracker commissioning crate ([0-2])" << std::endl;
		  tracker_crate = -1;
		}
	    }

	  else if (arg=="-h" || arg=="--help")
	    {
	      std::cout << std::endl;
	      std::cout << "Usage:   " << argv[0] << " [options]" << std::endl;
	      std::cout << std::endl;
	      std::cout << "Options:   -h / --help" << std::endl;
	      // std::cout << "           -i / --input  RED_FILE" << std::endl;
	      std::cout << "           -r / --run    RUN_NUMBER" << std::endl;
	      std::cout << "           -r / --event  EVENT_NUMBER" << std::endl;
	      std::cout << std::endl;
	      std::cout << "           -a / --tracker-area   [0-7]" << std::endl;
	      std::cout << "           -c / --tracker-crate  [0-2]" << std::endl;
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
	       "%s/snemo_run-%d_red-v2.data.gz", red_path, run_number);
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
  snfee::data::raw_event_data red;
    
  // RED counter
  std::size_t red_counter = 0;
  bool event_found = false;

  std::cout << "Searching for event " << event_number << " ..." << std::endl;

  while (red_source.has_record_tag())
    {
      // Check the serialization tag of the next record:
      DT_THROW_IF(!red_source.record_tag_is(snfee::data::raw_event_data::SERIAL_TAG),
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


      sndisplay::demonstrator *demonstrator_display = new sndisplay::demonstrator ("Demonstrator");
      demonstrator_display->setrange(0, 1);

      // scan calorimeter hits
      const std::vector<snfee::data::calo_digitized_hit> red_calo_hits = red.get_calo_hits();
      printf("\n=> %zd CALO HIT(s) :\n", red_calo_hits.size());

      for (const snfee::data::calo_digitized_hit & red_calo_hit : red_calo_hits)
	{
	  const snfee::data::timestamp & reference_time = red_calo_hit.get_reference_time();
	  const int64_t calo_tdc = reference_time.get_ticks();

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

	      printf("M:%d.%02d.%02d  (OM %3d)   TDC = %12ld   Amplitude = %5.1f mV   ",
		     om_side, om_column, om_row, om_num, calo_tdc, -calo_amplitude);
	    }

	  else if (om_id.is_xwall())
	    {
	      om_side   = om_id.get_side();
	      om_wall   = om_id.get_wall();
	      om_column = om_id.get_column();
	      om_row    = om_id.get_row();
	      om_num = 520 + om_side*64 +  om_wall*32 + om_column*16 + om_row;

	      printf("X:%d.%d.%d.%02d (OM %3d)   TDC = %12ld   Amplitude = %5.1f mV  ",
		     om_side, om_wall, om_column, om_row, om_num, calo_tdc, -calo_amplitude);
	    }

	  else if (om_id.is_gveto())
	    {
	      om_side = om_id.get_side();
	      om_wall = om_id.get_wall();
	      om_column = om_id.get_column();
	      om_num = 520 + 128 + om_side*32 + om_wall*16 + om_column;

	      printf("G:%d.%d.%02d   (OM %3d)   TDC = %12ld   Ampl = %5.1f mV  ",
		     om_side, om_wall, om_column, om_num, calo_tdc, -calo_amplitude);
	    }

	  printf("%s %s\n",
		 red_calo_hit.is_high_threshold() ? "[HT]" : "    ",
		 red_calo_hit.is_low_threshold_only() ? "[LT]" : "");

	  if (red_calo_hit.is_high_threshold() || red_calo_hit.is_low_threshold_only())
	    demonstrator_display->setomcontent(om_num, 1);
	}

      // scan tracker hits
      const std::vector<snfee::data::tracker_digitized_hit> red_tracker_hits = red.get_tracker_hits();
      printf("\n=> %zd TRACKER HIT(s) :\n", red_tracker_hits.size());

      for (const snfee::data::tracker_digitized_hit & red_tracker_hit : red_tracker_hits)
	{
	  const sncabling::gg_cell_id gg_id = red_tracker_hit.get_cell_id();

	  int cell_side  = gg_id.get_side();
	  int cell_row   = gg_id.get_row();
	  int cell_layer = gg_id.get_layer();
	  int cell_num = 113*9*cell_side + 9*cell_row + cell_layer;

	  const std::vector<snfee::data::tracker_digitized_hit::gg_times> & gg_timestamps_v = red_tracker_hit.get_times();

	  bool has_anode = false;
	  bool has_bottom_cathode = false;
	  bool has_top_cathode = false;
	  bool first_timestamp = true;

	  for (const snfee::data::tracker_digitized_hit::gg_times & gg_timestamps : red_tracker_hit.get_times())
	    {
	      if (first_timestamp)
		{
		  printf("GG:%d.%03d.%1d", cell_side, cell_row, cell_layer);
		  first_timestamp = false;
		}
	      else
		printf("          ");

	      for (int r=0; r<5; r++)
		{
		  const snfee::data::timestamp anode_timestamp = gg_timestamps.get_anode_time(r);
		  const int64_t anode_tdc = anode_timestamp.get_ticks();

		  if (anode_tdc != snfee::data::INVALID_TICKS)
		    {
		      printf("     R%d = %12lu", r, anode_tdc);
		      if (r == 0) has_anode = true;
		    }
		  else printf("                      ");
		}

	      {
		const snfee::data::timestamp bottom_cathode_timestamp = gg_timestamps.get_bottom_cathode_time();
		const int64_t bottom_cathode_tdc = bottom_cathode_timestamp.get_ticks();

		if (bottom_cathode_tdc != snfee::data::INVALID_TICKS)
		  {
		    printf("     R5 = %12lu", bottom_cathode_tdc);
		    has_bottom_cathode = true;
		  }
		else printf("                      ");
	      }

	      {
		const snfee::data::timestamp top_cathode_timestamp = gg_timestamps.get_top_cathode_time();
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

      std::string title = Form("RUN %d // EVENT %d // TRIGGER ID ", run_number, event_number);
      
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

      if (tracker_area != -1)
	{
	  // put in gray color unused cells
	  int first_row = 14*tracker_area;

	  if (tracker_area >= 4)
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

      else if (tracker_crate != -1)
	{
	  // put in gray color unused cells
	  const int crate_rows[4] = {0, 38, 75, 113};

	  int first_row = crate_rows[tracker_crate];
	  int last_row = crate_rows[tracker_crate+1];

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

