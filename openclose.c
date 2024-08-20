#include <u.h>
#include <libc.h>
#include <gio.h>

RWLock giolock;
ReadWriter **gio_filedes;
uchar *gio_filedes_st;
int gio_maxfile = 0;

int
gio_init(int maxfile)
{
	if(gio_maxfile > 0)
		return 0; // no-op if already initialized
	wlock(&giolock);
	if(!(gio_filedes = mallocz(sizeof(uintptr), nil)))
		goto fail;
	if(!(gio_filedes_st = mallocz(sizeof(uchar), nil)))
		goto fail;
	gio_maxfile = maxfile;
	wunlock(&giolock);
	return 0;
fail:
	free(gio_filedes_st);
	free(gio_filedes);
	wunlock(&giolock);
	return -1;
}

int
getnext(void)
{
	int i;
	for(i = 0; i < gio_maxfile; i++)
		if(gio_filedes_st[i] == 0)
			break;
	if(i == gio_maxfile)
		return -1;
	return i;
}

ReadWriter*
getrdstruct(int fd)
{
	ReadWriter *rval;
	rlock(&giolock);
	if(gio_filedes_st[fd] != 1){
		runlock(&giolock);
		return nil;
	}
	rval = gio_filedes[fd];
	runlock(&giolock);
	return rval;
}

int
gopen(ReadWriter* rd, void *aux)
{
	int pfd;
	ReadWriter *buf;

	if(!(gio_maxfile > 0))
		return -1;
	wlock(&giolock);
	pfd = getnext();
	if(pfd == -1){
		wunlock(&giolock);
		return -1;
	}
	buf = malloc(sizeof(ReadWriter));
	if(buf == nil)
		exits("bad malloc");
	memcpy(buf, rd, sizeof(ReadWriter));
	buf->aux = aux;
	buf->offset = 0;
	if(buf->open != nil){
		if((buf->open(buf)) != 0){
			buf->close(buf);
			free(buf);
			wunlock(&giolock);
			return -1;
		}
	}
	gio_filedes[pfd] = buf;
	gio_filedes_st[pfd] = 1;
	wunlock(&giolock);
	return pfd;
}

int
_gclose(int fd)
{
	ReadWriter *bf;
	int rval = 0;

	if(gio_filedes_st[fd] == 0)
		return -1;
	bf = gio_filedes[fd];
	if(bf->close != nil)
		rval = bf->close(bf);
	free(bf);
	gio_filedes_st[fd] = 0;
	gio_filedes[fd] = nil;
	return rval;
}

int
gclose(int fd)
{
	int rval = 0;

	// this check is done twice so users can avoid holding a lock
	// when they don't need to
	if(gio_filedes_st[fd] == 0)
		return -1;
	wlock(&giolock);
	rval = _gclose(fd);
	wunlock(&giolock);
	return rval;
}

int
gio_free(void)
{
	wlock(&giolock);
	for(int i = 0; i < gio_maxfile; i++)
		_gclose(i);
	free(gio_filedes_st);
	free(gio_filedes);
	wunlock(&giolock);
	return 0;
}
