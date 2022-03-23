#ifndef H5_FILEWRAPPER_H
#define H5_FILEWRAPPER_H
#define FUSE_USE_VERSION 31
#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <cstring>

#include <fuse.h>
#include <sys/stat.h>
#include <hdf5.h>

class H5FSRoot
{
public:
    static H5FSRoot& GetRootObject();
    H5FSRoot(H5FSRoot&& other) = delete;
    H5FSRoot& operator=(const H5FSRoot& other) = delete;

public:
    hid_t   OpenRoot(const std::string&& path);
    herr_t  CloseRoot();
    hid_t   GetRootHandle() const;
    herr_t  FillDir(const std::string&& path, std::vector<std::string>& entries);
    herr_t  GetAttr(const std::string&& path, struct stat *stbuf);
    herr_t  Open(const std::string&& path, struct fuse_file_info *fi);
    hsize_t Read(const std::string&& path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
    herr_t  Unlink(const std::string&& path);
    herr_t  Mkgrp(const std::string&& path, mode_t mode);

private:
    H5FSRoot() : m_rootid(-1) {};
    hsize_t DsetSize(const std::string&& path);
    hid_t m_rootid;
};

#endif /* H5_FILEWRAPPER_H */