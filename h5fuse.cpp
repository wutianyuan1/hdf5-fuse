#define FUSE_USE_VERSION 31

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#include "h5filewrapper.h"

static int fill_dir_plus = 0;
static auto& rootgrp = H5FSRoot::GetRootObject();

static void* h5fuse_init(struct fuse_conn_info *conn,
		      struct fuse_config *cfg){
	(void) conn;
	cfg->use_ino = 1;
	cfg->entry_timeout = 0;
	cfg->attr_timeout = 0;
	cfg->negative_timeout = 0;
	H5Eset_auto(H5E_DEFAULT, NULL, NULL);
	return NULL;
}

static int h5fuse_getattr(const char *path, struct stat *stbuf,
		       struct fuse_file_info *fi){
	(void) fi;
	return rootgrp.GetAttr(std::string(path), stbuf);
}


static int h5fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi,
		       enum fuse_readdir_flags flags){
	(void) offset;
	(void) fi;
	(void) flags;
	std::vector<std::string> contents;
	if (rootgrp.FillDir(std::string(path), contents) != 0)
		return ENOENT;
	for (auto&& dent : contents) {
		if (filler(buf, dent.c_str(), NULL, 0, (fuse_fill_dir_flags)fill_dir_plus))
			break;
	}
	return 0;
}

static int h5fuse_mkdir(const char *path, mode_t mode){
	return rootgrp.Mkgrp(std::string(path), mode);
}

static int h5fuse_unlink(const char *path){
	return rootgrp.Unlink(std::string(path));
}

static int h5fuse_open(const char *path, struct fuse_file_info *fi){
	return rootgrp.Open(std::string(path), fi);
}

static int h5fuse_read(const char *path, char *buf, size_t size, 
			off_t offset, struct fuse_file_info *fi){
	return rootgrp.Read(std::string(path), buf, size, offset, fi);
}

static int h5fuse_release(const char *path, struct fuse_file_info *fi){
	(void) path;
	return 0;
}

static int h5fuse_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi){
	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

static void h5fuse_destroy(void *private_data){
	(void) private_data;
	rootgrp.CloseRoot();
}

static const struct fuse_operations h5fuse_oper = {
	.getattr	= h5fuse_getattr,
	.mkdir		= h5fuse_mkdir,
	.unlink		= h5fuse_unlink,
	.open		= h5fuse_open,
	.read		= h5fuse_read,
	.release	= h5fuse_release,
	.fsync		= h5fuse_fsync,
	.readdir	= h5fuse_readdir,
	.init       = h5fuse_init,
	.destroy	= h5fuse_destroy,
};

int main(int argc, char *argv[]){
	enum { MAX_ARGS = 10 };
	int i,new_argc;
	char *new_argv[MAX_ARGS];

	umask(0);
	for (i = 0, new_argc = 0; (i < argc-1) && (new_argc < MAX_ARGS); i++) {
		if (!strcmp(argv[i], "--plus"))
			fill_dir_plus = FUSE_FILL_DIR_PLUS;
		else
			new_argv[new_argc++] = argv[i];
	}
	
	rootgrp.OpenRoot(argv[argc - 1]);
	return fuse_main(new_argc, new_argv, &h5fuse_oper, NULL);
}

