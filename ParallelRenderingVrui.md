#Parallel rendering in vrui

# Introduction #


Vrui itself uses a hybrid split-first / split-middle approach.  cluster-transparent.

  * At the coarsest level, Vrui uses replication and runs the mainloop code identically on all cluster nodes, synchronized by explicit broadcasting of input device and ancillary data.
  * On a finer level, Vrui uses split-middle approaches with explicit communication between cluster nodes in several components, sometimes provided by lower-level libraries. For example, Vrui relies on the split-middle distribution paradigm of the IO library for its own cluster-transparent file and network access.



Distribution abstraction using a split-first distribution of the toolkit itself, with support for transparent multicast communication between an application computer and the rendering computers to allow applications to internally use a split-middle scheme, for example using an external scene graph API or application-specific communication protocols. On a shared-memory multi-CPU/multi-pipe visualization system, applications should automatically make use of parallelism and high-speed communications.
, Vrui contains a comprehensive cluster-transparent file I/O handling library, explicit high-performance intra-cluster communication,


http://idav.ucdavis.edu/~okreylos/ResDev/Vrui/X11ClusterRendering.html