.TH GIO 2
.SH NAME
gio_init, gio_free, gopen, gclose, gseek, gread, gwrite, fd2gio \- Programmable streams
.SH SYNOPSIS
.B #include <u.h>
.br
.B #include <libc.h>
.br
.B #include <gio.h>
.PP
.ft L
.nf
typedef struct ReadWriter {
	RWLock;
	int (*open)(ReadWriter*);
	int (*close)(ReadWriter*);
	long (*pread)(ReadWriter*, void*, long, vlong);
	long (*pwrite)(ReadWriter*, void*, long, vlong);
	void *aux;
	u64int offset;
	u64int length;
} ReadWriter;
.fi
.PP
.B
int gio_init(int maxfile)
.PP
.B
int gio_free(void)
.PP
.B
int gopen(ReadWriter *r, void *aux)
.PP
.B
int gclose(int gfd)
.PP
.B
vlong gseek(int gfd, vlong offset, int whence)
.PP
.B
long gread(int gfd, void *buf, long nbytes, vlong offset)
.PP
.B
long gwrite(int gfd, void *buf, long nbytes, vlong offset)
.PP
.B
int fd2gio(int fd)
.SH DESCRIPTION
.I gio_init
initializes the libgio file descriptor table sizing it to
.I maxfiles.
.I gio_free
attempts to close all open files in the table and then frees the resources used by libgio.
.I gopen
takes a ReadWriter struct and creates a new instance of a gio fd.
.I aux
is an auxillary argument that may or may not be optional depending on how the gio stream is implemented.
.I gclose
closes a gio fd and frees any resources allocated to it.
.I gseek
changes the fd in a similar way to
.IR seek (2).
.I gread
and
.I gwrite
are the gio equivalents to
.IR pread (2)
and
.IR pwrite (2).
They are functionally equivalent and have nearly the same usage.
.I fd2gio
takes a Plan 9 file descriptor and returns a gio descriptor made from that fd.
.SH SOURCE
.B /sys/src/libgio
.SH NOTES
The gio functions automatically lock the ReadWriter struct in use hopefully making things work better with
.IR thread (2).
.SH BUGS
Things get interesting with
.IR rfork (2)
when you disable shared memory.
.br
The file descriptor table is poorly implemented.
.br
You cannot easily mix gio file descriptors with functions that want Plan 9 ones.
.br
There is only one file descriptor table.
.br
.I gseek
isn't really used by the library.
.br

.SH SEE ALSO
.IR read (2),
.IR seek (2),
.IR open (2)
.SH DIAGNOSTICS
By default, no. Users can have their methods set 
.I errstr,
and
.I gopen,
.I gclose,
.I gread,
and
.I gwrite
will return -2 if a function pointer is nil, and
.I gio_init
and
.I gio_free
will return -1 if something bad happens.
.SH HISTORY
First appeared as part of
.IR ngfs (8)
in September 2015
