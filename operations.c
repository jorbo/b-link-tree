#include "operations.h"
#include "node.h"
#include "search.h"
#include "insert.h"


Request encode_search_req(search_in_t in) {
	Request req = {
		.opcode = SEARCH,
		{ .search = in }
	};
	return req;
}

Request encode_insert_req(insert_in_t in) {
	Request req = {
		.opcode = INSERT,
		{ .insert = in }
	};
	return req;
}

Response encode_search_resp(search_out_t out) {
	Response resp = {
		.opcode = SEARCH,
		{ .search = out }
	};
	return resp;
}

Response encode_insert_resp(insert_out_t out) {
	Response resp = {
		.opcode = INSERT,
		{ .insert = out }
	};
	return resp;
}


Response execute_req(Request req, bptr_t *root, mem_context_t *ctx HBM_PARAM) {
	Response resp = {.opcode = req.opcode};
	switch (req.opcode) {
		case NOP: break;
		case SEARCH:
			resp.search = search(*root, req.search, ctx HBM_ARG);
			break;
		case INSERT:
			resp.insert = insert(root, req.insert.key, req.insert.value, ctx HBM_ARG);
			break;
	}
	return resp;
}
