// Ourselves:
#include "calo_histogramming.h"

// This project:
#include <snfee/data/calo_waveform_drawer.h>
#include <snfee/model/feb_constants.h>

namespace snfee {
  namespace calo {

    histogramming::histogramming(const config_type & cfg_)
    {
      config = cfg_;
      hservice_config.store("root_export.stats", true);
      return;
    }
    
    void histogramming::initialize()
    {
      hservice.initialize_standalone(hservice_config);
      hpool = &hservice.grab_pool();
      tree_name = "RTD calo histograms";
      return;
    }

    void histogramming::terminate()
    {
      hservice.store_as_root_file(config.root_output_filename);
      hservice.reset();
      return;
    }

    void histogramming::fill(const std::string & ch_id_str_,
                             const int run_id_,
                             const std::string & label_,
                             const double        value_,
                             const double        value2_)
          {
      // Histogram name:
      std::ostringstream h_name_s;
      h_name_s << "h" << label_ << "_" << ch_id_str_;
      std::string h_name = h_name_s.str();

      // Histogram title:
      std::ostringstream h_title_s;
      h_title_s << "Calo hit " << label_;
      if (run_id_ >= 0) {
        h_title_s << " - Run ID: " << run_id_;
      }
      if (!ch_id_str_.empty()) {
        h_title_s << " - Channel: " << ch_id_str_;
      }

      if (label_ == "peak_charge") {
        // 2D-histograms:
        if (!this->hpool->has_2d(h_name)) {
          
          mygsl::histogram_2d & h2 = this->hpool->add_2d(h_name, h_title_s.str(), this->tree_name);
          if (label_ == "peak_charge") {
            h2.initialize(this->config.histo_peak_nbins,
                          this->config.histo_peak_min,
                          this->config.histo_peak_max,
                          this->config.histo_charge_nbins,
                          this->config.histo_charge_min,
                          this->config.histo_charge_max);
          }
        }
        mygsl::histogram_2d & h2 = hpool->grab_2d(h_name);
        h2.fill((double) value_, (double) value2_);
       
      } else {
        // 1D-histograms:
      
        if (!this->hpool->has_1d(h_name)) {
          mygsl::histogram_1d & h = this->hpool->add_1d(h_name, h_title_s.str(), this->tree_name);
          if (label_ == "charge") {
            h.initialize(this->config.histo_charge_nbins,
                         this->config.histo_charge_min,
                         this->config.histo_charge_max);
            // h.grab_auxiliaries().store("display.xaxis.unit", "nV.s");
          }
          if (label_ == "peak") {
            h.initialize(this->config.histo_peak_nbins,
                         this->config.histo_peak_min,
                         this->config.histo_peak_max);
          }
          if (label_ == "baseline") {
            h.initialize(this->config.histo_baseline_nbins,
                         this->config.histo_baseline_min,
                         this->config.histo_baseline_max);
          }
        }
        mygsl::histogram_1d & h = hpool->grab_1d(h_name);
        h.fill((double) value_);
      }
      return;
    }
  
  } // namespace calo
} // namespace snfee
