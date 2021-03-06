/**
 * $Id: sptask.h 815 2014-12-15 13:40:52Z klugeflo $
 * @file sptask.h
 * @brief Sporadic periodic task class
 * @author Florian Kluge <kluge@informatik.uni-augsburg.de>
 */

#ifndef TASKMODELS_SPTASK_H
#define TASKMODELS_SPTASK_H 1

#include <core/scobjects.h>

namespace tmssim {

  class SPTask : public Task {
  public:
    SPTask(unsigned int _id, int _period, int _et, unsigned int _seed,
	   int _ct, UtilityCalculator* __uc, UtilityAggregator* __ua,
	   double _probability=0.5, int _offset=0, int _prio=1);
    SPTask(unsigned int _id, int _period, int _et, unsigned int _seed,
	   UtilityCalculator* __uc, UtilityAggregator* __ua, int _prio);
    SPTask(const SPTask &rhs);
    
    virtual std::ostream& print(std::ostream& ost) const;
    
    virtual double getLastExecValue(void) const;
    virtual double getHistoryValue(void) const;
    //virtual double getPossibleExecValue(const Job *job, int startTime) const;
    //virtual double getPossibleHistoryValue(const Job *job, int startTime) const;
    
    /**
     * Serializes this object with the given xmlTextWriter to a xml-document
     * @param writer A pointer to the xmlWriter that should be used for serialization
     */
    //void write(xmlTextWriterPtr writer) const;
    
    /**
     * Creates a deep-copy of the current task
     * @return A pointer to a new object with the same properties of this object
     */
    virtual Task* clone() const;

    /**
     * @name XML
     * @{
     */
    virtual const std::string& getClassElement();
    virtual int writeData(xmlTextWriterPtr writer);
    /**
     * @}
     */
    
  protected:
    virtual int startHook(int now);
    virtual Job* spawnHook(int now);
    virtual int getNextActivationOffset(int now);
    virtual bool completionHook(Job *job, int now);
    virtual bool cancelHook(Job *job);
    virtual double calcExecValue(const Job *job, int complTime) const;
    virtual double calcHistoryValue(const Job *job, int complTime) const;
    virtual double calcFailHistoryValue(const Job *job) const;
    
    virtual void advanceHistory(void);
    
    virtual char getShortId(void) const;
    
    int period;
    int offset; ///< offset for first activation
    double lastUtility;
    double historyUtility;
    bool activationPending;
    unsigned int seed;
    double probability;
    unsigned int baseSeed;
    //int _priority;
  };

} // NS tmssim  

#endif /* !TASKMODELS_SPTASK_H */
