#ifndef OPERATIONS_H
#define OPERATIONS_H


#include "types.h"


typedef struct Node Node;
typedef struct mem_context mem_context_t;


/* Use fixed-width type to ensure consistent ABI between C and C++ compilation units */
typedef uint32_t Opcode;
#define NOP    ((Opcode)0)
#define SEARCH ((Opcode)1)
#define INSERT ((Opcode)2)

typedef struct {
	Opcode opcode;
	union {
		search_in_t search;
		insert_in_t insert;
	};
} Request;

typedef struct {
	Opcode opcode;
	union {
		search_out_t search;
		insert_out_t insert;
	};
} Response;


Request encode_search_req(search_in_t in);
Request encode_insert_req(insert_in_t in);
Response encode_search_resp(search_out_t out);
Response encode_insert_resp(insert_out_t out);

Response execute_req(Request req, bptr_t *root, mem_context_t *ctx);


#endif
