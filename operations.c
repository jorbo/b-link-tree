#include "operations.h"
#include "node.h"
#include "search.h"
#include "insert.h"


Request encode_search_req(search_in_t in) {
	Request req = {};
	memset(&req, 0, sizeof(req));
	req.opcode = SEARCH;
	req.search = in ;
	
	return req;
}

Request encode_insert_req(insert_in_t in) {
	Request req = {};
	memset(&req, 0, sizeof(req));
	req.opcode = INSERT;
	req.insert = in ;
}

Response encode_search_resp(search_out_t out) {
	Response resp = {};
	memset(&resp, 0, sizeof(resp));
	resp.opcode = SEARCH;
	resp.search = out ;
	return resp;
}

Response encode_insert_resp(insert_out_t out) {
	Response resp = {};
	memset(&resp, 0, sizeof(resp));
	resp.opcode = INSERT;
	resp.insert = out ;
	return resp;
}


Response execute_req(Request req, bptr_t *root, Node **memory) {
	Response resp = {};
	resp.opcode = req.opcode;
	switch (req.opcode) {
		case NOP: break;
		case SEARCH:
			resp.search = search(*root, req.search, memory);
			break;
		case INSERT:
			resp.insert = insert(root, req.insert.key, req.insert.value, memory);
			break;
	}
	return resp;
}
