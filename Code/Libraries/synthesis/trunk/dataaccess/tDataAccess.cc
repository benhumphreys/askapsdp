#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Utilities/Assert.h>

#include <iostream.h>

#include <boost/shared_ptr.hpp>

//#include <METableDataAccessor.h>
#include "IDataSelector.h"
#include "MEDataIterator.h"
#include "MEDataSource.h"

using namespace conrad;
using namespace boost;
using namespace casa;

/// We don't yet have a valid implementation of the interfaces.
/// Therefore all operations have been collected inside a function 
/// (we can use just the interface here to check whether it compiles)

void doTest(const shared_ptr<MEDataSource> &ds) {
     AlwaysAssert((Bool)ds,AipsError);
     
     // obtain and configure data selector
     shared_ptr<IDataSelector> sel=ds->createSelector();   
     sel->chooseChannels(100,150); // 100 channels starting from 150
     sel->chooseStokes("IQUV");

     // get the iterator
     shared_ptr<MEDataIterator> it=ds->createIterator(sel);

     // don't need it->init() the first time, although it won't do any harm
     for (;it->hasMore();it->next()) {
         cout<<"Block has "<<(*it)->nRow()<<" rows"<<endl; 
	 // an alternative way of access
	 const MEDataAccessor &da=*(*it);
	 cout<<"Number of channels: "<<da.nChannel()<<endl; // should be 100
     }
}
 
int main() {
	
    try {
       // nothing at this stage, we just check that the code is
       // compilable
		
    } catch (AipsError x) {
        cout << "Caught an exception: " << x.getMesg() << endl;
        return 1;
    } 
	return 0;
}

