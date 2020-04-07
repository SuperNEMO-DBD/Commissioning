// Ourselves:
#include "calo_waveform_fft.h"

// Third party:
// - Bayeux:
#include <bayeux/mygsl/fft_real.h>
#include <bayeux/datatools/exception.h>
#include <bayeux/geomtools/geomtools_config.h>
#if GEOMTOOLS_WITH_GNUPLOT_DISPLAY == 1
#include <bayeux/geomtools/gnuplot_i.h>
#include <bayeux/geomtools/gnuplot_drawer.h>
#endif // GEOMTOOLS_WITH_GNUPLOT_DISPLAY
#include <bayeux/datatools/temporary_files.h>


// This project:
#include <snfee/data/calo_waveform_data.h>
#include <snfee/model/feb_constants.h>

namespace snfee {
  namespace algo {

    calo_waveform_fft::calo_waveform_fft(const config_type & cfg_,
                                         const datatools::logger::priority logging_)
    {
      logging = logging_;
      _config_ = cfg_;
      return;
    }

    void calo_waveform_fft::initialize()
    {
      return;
    }
    
    void calo_waveform_fft::terminate()
    {
       _config_ = config_type();
      return;
    }

    void calo_waveform_fft::transform(const snfee::data::calo_waveform & wf_,
                                      std::vector<double> & ft_,
                                      std::vector<double> & fwf_,
                                      double & frequency_step_)
    {
      DT_THROW_IF(!wf_.is_locked(), std::logic_error, "Waveform is not locked!");
      double time_step = wf_.get_times_ns()[1] - wf_.get_times_ns()[0];
      double Nyquist_frequency = 0.5 / time_step;
      double min_freq_cut = 0.0;
      double max_freq_cut = Nyquist_frequency;
      double time_min = wf_.get_times_ns()[0];
      std::vector<double> data(wf_.get_amplitudes_mV().size());
      std::copy(wf_.get_amplitudes_mV().begin(),
                wf_.get_amplitudes_mV().end(),
                data.begin());
      mygsl::fft_real fft;
      fft.init(data, time_min, time_step, min_freq_cut, max_freq_cut);
      fft.process();
      fft.compute_fourier_spectrum(ft_);
      fft.compute_filtered_data(fwf_);
      frequency_step_ = fft.get_frequency_step();
      return;
    }

    // static
    void calo_waveform_fft::display_waveform_fft(const std::vector<double> & ft_,
                                                 const double frequency_step_GHz_,
                                                 const std::string & title_)
    {
      datatools::temp_file ftmp;
      ftmp.set_remove_at_destroy(true);
      ftmp.create("/tmp", "snfee_algo_calo_waveform_fft-draw-temp_");
      std::string ftmp_path = ftmp.get_filename();
      for (int i = 0; i < (int) ft_.size(); i++) {
        double f_GHz = i * frequency_step_GHz_;
        double t = ft_[i];
        ftmp.out() << f_GHz << ' ' << t << std::endl;
      }

#if GEOMTOOLS_WITH_GNUPLOT_DISPLAY == 1
      Gnuplot g1;
      std::string title = title_;
      std::string terminal_id = "x11";
      g1.cmd("set terminal " + terminal_id); 
      g1.cmd("set encoding utf8");
      g1.cmd("set title '" + title + "'");
      g1.cmd("set grid");
      g1.cmd("set xlabel 'Frequency (GHz)'");
      g1.cmd("set ylabel 'Fourier amplitude'");
      double min_frequency = 0.0;
      double max_frequency = ft_.size() * frequency_step_GHz_ / 4;
      g1.cmd("set xrange [" + std::to_string(min_frequency) + ":" + std::to_string(max_frequency) + "]");
      // g1.cmd("set yrange [" + std::to_string(amp_min_mV)  + ":" + std::to_string(amp_max_mV) + "]");

      std::string sample_color = "rgb '#0090ff'";
      std::ostringstream plot_cmd;
      plot_cmd << "plot ";
      plot_cmd << "'" << ftmp.get_filename()
               << "' notitle with impulses lt " << sample_color << " lw 1 ";
      g1.cmd(plot_cmd.str());
      g1.showonscreen(); // window output
      geomtools::gnuplot_drawer::wait_for_key();
      usleep(200);
#endif // GEOMTOOLS_WITH_GNUPLOT_DISPLAY == 1
      return;
    }

  } // namespace algo
} // namespace snfee
