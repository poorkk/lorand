
/*
 * Relation --------------------------------------------------------
 */
typedef struct {

} RelFile;

/*
 * Relation --------------------------------------------------------
 */

typedef int BufferId;

typedef struct {

} HashCache;

BufferId hashcache_read(HashCache *hc, int a);

BufferId buffer_read(RelFile rel, int pagenum);
BufferId buffer_extend_read(RelFile rel, int pagenum);