#ifndef MUELU_MONITOR_HPP
#define MUELU_MONITOR_HPP

#include <string>
#include <Teuchos_DefaultComm.hpp>
#include <Teuchos_Time.hpp>
#include <Teuchos_TimeMonitor.hpp>
#include "MueLu_ConfigDefs.hpp"
#include "MueLu_BaseClass.hpp"
#include "MueLu_VerboseObject.hpp"

namespace MueLu {

  // Helper function. Similar to Teuchos::TimeMonitor::summarize().
  ArrayRCP<double> ReduceMaxMinAvg(double localValue, Teuchos::Comm<int> const &comm, int rootNode = 0);

  class MonitorBase : public BaseClass {

  public:

    MonitorBase(const std::string& descr, VerbLevel verbLevel, const RCP<Teuchos::FancyOStream> & out, bool runTimer = true)
      : descr_(descr)
    {

      SetVerbLevel(verbLevel);
      setOStream(out);
      
      // Print descr
      GetOStream(Runtime0, 0) << descr_ << std::endl;
      // verbObject.describe(getOStream(), getVerbLevel()); //TODO
      
      tab_ = rcp(new Teuchos::OSTab(getOStream()));
      
      if (runTimer) {
        // Start the timer
        timer_ = Teuchos::TimeMonitor::getNewTimer("MueLu: " + descr_);
        timeMonitor_ = rcp(new Teuchos::TimeMonitor(*timer_));
      }
    }

    ~MonitorBase() {
      if (timer_ != Teuchos::null) {

        // Stop the timer
        timeMonitor_ = Teuchos::null;
        
        if (IsPrint(RuntimeTimings)) {
          //FIXME: creates lot of barriers. An option to report time of proc0 only instead would be nice
          //FIXME: MPI_COMM_WORLD only... BTW, it is also the case in Teuchos::TimeMonitor...
          ArrayRCP<double> stats = ReduceMaxMinAvg(timer_->totalElapsedTime(), *Teuchos::DefaultComm<int>::getComm ());
          
          //FIXME: Not very important for now, but timer will be printed even if verboseLevel of Monitor/Object changed
          //       between Monitor constructor and destructor.
          tab_ = Teuchos::null;
          if (GetProcRankVerbose() == 0)
            *getOStream() << "... time: " << " max=" << stats[0] << " min=" << stats[1] << " avg=" << stats[2] << std::endl;
        }
      }
    }

  protected:
    MonitorBase() { }
    
  private:
    std::string descr_;

    RCP<Teuchos::Time> timer_; // keep a reference on the timer to print stats if RuntimeTimings=ON
    RCP<Teuchos::TimeMonitor> timeMonitor_;

    RCP<Teuchos::OSTab> tab_;
  };
  
  // Main Monitor
  // - A timer is created only if Timings0 == true
  // - Timer is printed on the go if RuntimeTimings)) == true
  class Monitor: public MonitorBase {
  public:
    Monitor(const BaseClass& object, const std::string & descr) 
      : MonitorBase(descr + " (" + object.description() + ")", object.GetVerbLevel(), object.getOStream(), object.IsPrint(Timings0))
    { }
  };

  // Another version of the Monitor, that does not repeat the object descr.
  // A timer is created only if Timings1 == true
  class SubMonitor: public MonitorBase {
  public:
    SubMonitor(const VerboseObject& object, const std::string & descr, MsgType timerLevel = Timings1)
      : MonitorBase(descr, object.GetVerbLevel(), object.getOStream(), object.IsPrint(Timings1))
    { }
  };
  
} // namespace MueLu

#endif // MUELU_MONITOR_HPP
