#ifndef SNFEE_ALGO_CALO_WAVEFORM_FFT_H
#define SNFEE_ALGO_CALO_WAVEFORM_FFT_H

// Standard library:
#include <cstdint>
#include <vector>
#include <limits>

// Third party:
// - Bayeux:
#include <bayeux/datatools/logger.h>

// This project:
#include <snfee/data/calo_waveform_data.h>

namespace snfee {
  namespace algo {
 
    /// \brief Calo waveform FFT
    struct calo_waveform_fft
    {

      /// \brief Configuration parameters
      struct config_type
      {
      };

      /// Constructor
      calo_waveform_fft(const config_type & cfg_,
                        const datatools::logger::priority logging_ = datatools::logger::PRIO_FATAL);

      /// Initialize
      void initialize();

      /// Terminate
      void terminate();

      /// Fourier transform
      void transform(const snfee::data::calo_waveform & wf_,
                     std::vector<double> & ft_,
                     std::vector<double> & fwf_,
                     double & frequency_step_);

      /// Display waveform FFT
      static void display_waveform_fft(const std::vector<double> & ft_,
                                       const double frequency_step_,
                                       const std::string & title_ = "");
      
      datatools::logger::priority logging = datatools::logger::PRIO_FATAL; ///< Logging priority threshold:
      
    private:

      config_type _config_;
      
    };
    
  } // namespace algo
} // namespace snfee

#endif // SNFEE_ALGO_CALO_WAVEFORM_FFT_H

// Local Variables: --
// mode: c++ --
// c-file-style: "gnu" --
// tab-width: 2 --
// End: --
