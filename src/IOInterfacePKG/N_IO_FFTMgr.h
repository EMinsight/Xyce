//-------------------------------------------------------------------------
//   Copyright 2002-2021 National Technology & Engineering Solutions of
//   Sandia, LLC (NTESS).  Under the terms of Contract DE-NA0003525 with
//   NTESS, the U.S. Government retains certain rights in this software.
//
//   This file is part of the Xyce(TM) Parallel Electrical Simulator.
//
//   Xyce(TM) is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Xyce(TM) is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Xyce(TM).
//   If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------

//----------------------------------------------------------------
//
// Purpose       : This file is a class to manage analysis objects
//                 that are generated by .FFT statements.
//
// Special Notes :
//
// Creator       : Pete Sholander, SNL
//
// Creation Date : 1/4/2021
//---------------------------------------------------------------

#ifndef  Xyce_N_IO_FFTMgr_H
#define Xyce_N_IO_FFTMgr_H

#include <list>
#include <string>

#include <Teuchos_SerialDenseMatrix.hpp>
#include <Teuchos_RCP.hpp>

#include <N_ANP_StepEvent.h>

#include <N_IO_fwd.h>
#include <N_LAS_Vector.h>
#include <N_PDS_fwd.h>
#include <N_TIA_StepErrorControl.h>

#include <N_UTL_fwd.h>
#include <N_UTL_Listener.h>
#include <N_UTL_Param.h>

namespace Xyce {
namespace IO {

typedef std::vector<FFTAnalysis *> FFTAnalysisVector;

//-----------------------------------------------------------------------------
// Class         : FFTMgr
// Purpose       : This is a manager class for handling analysis objects
//                 generated by .fft statements
// Special Notes :
// Creator       : Pete Sholander, SNL
// Creation Date : 1/4/2021
//-----------------------------------------------------------------------------
class FFTMgr : public Util::Listener<Analysis::StepEvent>
{
public:
  FFTMgr(const CmdParse &cp);

  // Destructor
  ~FFTMgr();

  void notify(const Analysis::StepEvent &step_event);

  // register options from .OPTIONS FFT lines
  bool registerFFTOptions(const Util::OptionBlock & option_block);
  bool getfft_accurate() const { return fft_accurate_; }

  // Return true if FFT analysis is being performed on any variables.
  bool isFFTActive() const { return fftAnalysisEnabled_ && !FFTAnalysisList_.empty(); }

  const FFTAnalysisVector& getFFTAnalysisList() const {return FFTAnalysisList_;}

  // add .fft line from netlist to list of things to perform analysis on.
  bool addFFTAnalysis(const Util::OptionBlock & fftLine );

  void enableFFTAnalysis(const Analysis::Mode analysisMode);
  void fixupFFTParameters(Parallel::Machine comm,
                          const IO::OutputMgr &output_manager,
                          const Util::Op::BuilderManager &op_builder_manager,
			  const double endSimTime,
                          TimeIntg::StepErrorControl & sec);
  void fixupFFTParametersForRemeasure(Parallel::Machine comm,
                          const Util::Op::BuilderManager &op_builder_manager,
			  const double endSimTime,
                          TimeIntg::StepErrorControl & sec);

  void resetFFTAnalyses();

  // Called during the simulation to update the fft objects held by this class
  void updateFFTData(Parallel::Machine comm,
                     const double circuitTime,
                     const Linear::Vector *solnVec,
                     const Linear::Vector *stateVec,
                     const Linear::Vector * storeVec,
                     const Linear::Vector *lead_current_vector,
                     const Linear::Vector *junction_voltage_vector,
                     const Linear::Vector *lead_current_dqdt_vector);

  void outputResultsToFFTfile(int stepNumber);
  void outputResults( std::ostream& outputStream );
  void outputVerboseResults(std::ostream& outputStream);

  //added to help register lead currents with device manager
  std::set<std::string> getDevicesNeedingLeadCurrents() { return devicesNeedingLeadCurrents_; }

private:
  const IO::CmdParse &  commandLine_;        // Command line
  bool                  fftAnalysisEnabled_; // set based on the analysis mode
  bool                  fft_accurate_;  // comes from .OPTIONS FFT line
  bool                  fftout_;        // comes from .OPTIONS FFT line
  int                   fft_mode_;      // comes from .OPTIONS FFT line
  FFTAnalysisVector     FFTAnalysisList_;

  //added to help register lead currents with device manager
  std::set<std::string> devicesNeedingLeadCurrents_;
};

bool registerPkgOptionsMgr(FFTMgr &fft_manager, PkgOptionsMgr &options_manager);

} // namespace IO
} // namespace Xyce

#endif  // Xyce_N_IO_FFTMgr_H
