#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"


class lt_initiator : public sc_core::sc_module
{
public:
    // Socket
    tlm_utils::simple_initiator_socket<lt_initiator> socket;
    // Payload
    tlm::tlm_generic_payload *payload;
    // Module's behaviour/process
    void behavior();
    // Constructor
    lt_initiator( sc_core::sc_module_name name);
    
    
private:
    // Status of completed transaction
    tlm::tlm_response_status status;
    // Location of data to be sent with a transaction
    unsigned char *data;
    
};
