/* --- file: Include/cpython/clgc.h --- */
#ifndef Py_CLGC_H
#define Py_CLGC_H
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GCINFO_PHASE_START = 0,         // Start of collection (no values marked)
    GCINFO_PHASE_MARK = 1,          // End of marking phase (leases renewed)
    GCINFO_PHASE_ANALYSIS = 2,      // End of reachability phase, managers to examine for relationships 
    GCINFO_PHASE_RESCUE = 3,  // End of reachability phase, items can be rescued
    GCINFO_PHASE_COLLECT = 4,       // Start of collection phase (leases broken)
    GCINFO_PHASE_DONE = 5           // Collection complete (audit phase)
} GCInfoPhase;


/*
 * Internal structure describing the state and hooks for a single phase of
 * garbage collection. This is passed to reference managers during collection.
 */
typedef struct _gc_manager_info {
    /*
     * The generation currently being collected.
     * Typically, only the highest (third) generation is of interest,
     * but multi-generation implementations are possible.
     */
    int gc_generation;

    /*
     * The current phase of the collection process:
     *   0 - Start of collection (no values are marked yet)
     *   1 - End of marking phase (leases are renewed until phase 2)
     *   2 - End of reachability phase (items can be rescued, leases transferred)
     *   3 - Start of collection phase (leases are broken until phase 4)
     *   4 - Collection complete (or start of audit phase)
     */
    GCInfoPhase gc_phase;

    /*
     * The visit method to be executed at this phase of the collection.
     * Used to traverse or process objects.
     */
    visitproc visit;
    void* visitargs;

    /*
     * Query function to determine if an object is collectable in this phase.
     * May be NULL if not available for the current phase.
     */
    inquiry is_collectable;

    /*
     * Optional hook to traverse the object tree and analyze collections.
     * Only set (non-NULL) during phase 2.
     */
    traverseproc traverse;
} _PyGCInfo;

/*
 * Reference manager callback type. Called during garbage collection.
 * Arguments:
 *   - info: Description of the current GC phase and available hooks.
 *   - args: An extension object which will function as the parent for references.
 * Returns 0 on success, nonzero on error.
 */
typedef int (*_gc_managerproc)(struct _gc_manager_info*, PyObject* args);

/* Public typedefs for external use */
typedef _PyGCInfo GCInfo;
typedef _gc_managerproc gc_managerproc;

/*
 * Register a reference manager callback with the CPython garbage collector.
 * Reference managers are called in the order they are installed (FIFO).
 * Arguments:
 *   - manager: Callback function to be invoked during collection.
 *   - resources: Opaque pointer, passed to the callback.
 */
PyAPI_FUNC(void) PyUnstable_GC_InstallReferenceManager(gc_managerproc manager, PyObject* resources);

/*
 * Remove a previously registered reference manager from the garbage collector.
 * Arguments:
 *   - manager: Callback function to remove.
 *   - resources: Opaque pointer, must match what was used at installation.
 */
PyAPI_FUNC(void) PyUnstable_GC_RemoveReferenceManager(gc_managerproc manager, PyObject* resources);

#ifdef __cplusplus
}
#endif
#endif /* !Py_CLGC_H */
