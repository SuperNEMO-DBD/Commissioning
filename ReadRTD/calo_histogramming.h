#ifndef CALO_HISTOGRAMMING_H
#define CALO_HISTOGRAMMING_H

// Standard library:
#include <cstdint>
#include <vector>
#include <limits>

// Third party:
// - Bayeux:
#include <bayeux/datatools/logger.h>
#include <bayeux/dpp/histogram_service.h>
#include <bayeux/mygsl/tabulated_sampling.h>
#include <bayeux/mygsl/tabulated_function.h>

// This project:
#include <snfee/data/channel_id.h>
#include <snfee/data/calo_hit_record.h>
#include <snfee/algo/calo_waveform_tools.h>

namespace snfee {
  namespace calo {

    /// \brief Calo waveform histogramming
    struct histogramming
    {

      /// \brief Configuration parameters
      struct config_type
      {
        /// Output Root filename
        std::string root_output_filename = "rtd_calo_histos.root";

        /// Use metadata from firmware in place of waveform off-line computations
        bool     histo_from_firmware  = false;
        
        // Histograms activation:
        bool     histo_peak           = true;
        bool     histo_charge         = true;
        bool     histo_peak_charge    = true;
        bool     histo_baseline       = true;

        // Histograms setup:
        uint16_t histo_charge_nbins   =  500;
        double   histo_charge_min     = -12.0; // nV.s
        double   histo_charge_max     =  +2.0; // nV.s

        uint16_t histo_peak_nbins     =  1300;
        double   histo_peak_min       = -1250.0;  // mV
        double   histo_peak_max       =   +50.0;  // mV

        uint16_t histo_baseline_nbins =  200;
        double   histo_baseline_min   = -10.0;   // mV
        double   histo_baseline_max   = +10.0;   // mV

      };

      /// Constructor
      histogramming(const config_type & cfg_);

      /// Initialize
      void initialize();

      /// Terminate
      void terminate();

      /// Fill a value for a given channel ID
      void fill(const std::string & ch_id_str_,
                const int           run_id_,
                const std::string & label_,
                const double        value_,
                const double        value2_ = std::numeric_limits<double>::quiet_NaN());
      
      datatools::logger::priority logging = datatools::logger::PRIO_FATAL; ///< Logging priority threshold:
      config_type             config;          ///< Configuration
      datatools::properties   hservice_config; ///< Configuration of the histogram service
      dpp::histogram_service  hservice;        ///< Histogram service
      mygsl::histogram_pool * hpool = nullptr; ///< Histogram pool handle
      std::string             tree_name;       ///< Root tree name
      
    };
    
  } // namespace calo
} // namespace snfee

#endif // CALO_HISTOGRAMMING_H

// Local Variables: --
// mode: c++ --
// c-file-style: "gnu" --
// tab-width: 2 --
// End: --
