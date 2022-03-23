#include "h5filewrapper.h"

hid_t
H5FSRoot::OpenRoot(const std::string&& path){
    if (!H5Fis_hdf5(path.c_str()))
        assert(false);
    m_rootid = H5Fopen(path.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
    if (m_rootid < 0)
        assert(false);
    return m_rootid;
}

herr_t
H5FSRoot::CloseRoot(){
    if (m_rootid >= 0)
        return H5Fclose(m_rootid);
    return 0;
}

H5FSRoot&
H5FSRoot::GetRootObject(){
    static std::unique_ptr<H5FSRoot> inst{new H5FSRoot()};
    return *inst;
}

hid_t 
H5FSRoot::GetRootHandle() const{
    return m_rootid;
}

hsize_t
H5FSRoot::DsetSize(const std::string&& path){
    hid_t dataset = H5Dopen(m_rootid, path.c_str(), H5P_DEFAULT);
    if(dataset < 0)
        return 0;

    hid_t datatype = H5Dget_type(dataset);
    hid_t dataspace = H5Dget_space(dataset);
    size_t type_size = H5Tget_size(datatype);
    size_t num_elems = H5Sget_simple_extent_npoints(dataspace);
    H5Sclose(dataspace);
    H5Dclose(dataset);
    return num_elems * type_size;
}

herr_t 
H5FSRoot::FillDir(const std::string&& path, std::vector<std::string>& entries){
    H5G_info_t group_info;
    assert(m_rootid >= 0);

    if(H5Gget_info_by_name(m_rootid, path.c_str(), &group_info, H5P_DEFAULT) < 0)
        return -ENOENT;
    entries.emplace_back(".");
    entries.emplace_back("..");

    for (hsize_t i = 0; i < group_info.nlinks; ++i) {
        char name[128];
        herr_t err =  H5Lget_name_by_idx(m_rootid, path.c_str(),
                        H5_INDEX_NAME, H5_ITER_INC, i, name, 128, H5P_DEFAULT);
        if (err < 0)
            return err;
        entries.emplace_back(std::string(name));
    }
    return 0;
}

herr_t
H5FSRoot::GetAttr(const std::string&& path, struct stat *stbuf){
    memset(stbuf, 0, sizeof(struct stat));
    H5O_info_t obj_info;
    assert(m_rootid >= 0);

    if(H5Oget_info_by_name(m_rootid, path.c_str(), &obj_info, H5O_INFO_ALL, H5P_DEFAULT) < 0)
        return -ENOENT;

    stbuf->st_ctim.tv_sec = obj_info.ctime;
    stbuf->st_atim.tv_sec = obj_info.atime;
    stbuf->st_mtim.tv_sec = obj_info.mtime;
    
    if(obj_info.type == H5O_TYPE_GROUP) {
        stbuf->st_mode = S_IFDIR | 0555;
        H5G_info_t group_info;
        H5Gget_info_by_name(m_rootid, path.c_str(), &group_info, H5P_DEFAULT);
        stbuf->st_nlink = 2 + group_info.nlinks;
        stbuf->st_size = group_info.nlinks;
    }
    else if (obj_info.type == H5O_TYPE_DATASET) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_size = DsetSize(std::move(path));
    }
    else {
        stbuf->st_mode = S_IFCHR | 0000;
        stbuf->st_size = 0;
    }
    return 0;
}

herr_t
H5FSRoot::Open(const std::string&& path, struct fuse_file_info *fi){
    if((fi->flags & 3) != O_RDONLY)
        return -EACCES;
    H5O_info_t obj_info;
    if(H5Oget_info_by_name(m_rootid, path.c_str(), &obj_info, H5P_DEFAULT, H5P_DEFAULT) < 0)
        return -ENOENT;
    fi->fh = H5Dopen(m_rootid, path.c_str(), H5P_DEFAULT);
    return 0;
}

hsize_t
H5FSRoot::Read(const std::string&& path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    (void) fi;
    hid_t dataset = H5Dopen(m_rootid, path.c_str(), H5P_DEFAULT);
    hid_t datatype = H5Dget_type(dataset);
    size_t buf_size = DsetSize(path.c_str());
    char *hdf5_buf = new char[buf_size];
    H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, hdf5_buf);
    size_t copy_size = buf_size - offset < size ? buf_size - offset : size;
    memcpy(buf, hdf5_buf+offset, copy_size);
    delete[] hdf5_buf;
    H5Dclose(dataset);
    return copy_size;
}

herr_t
H5FSRoot::Unlink(const std::string&& path){
    H5O_info_t obj_info;
    if(H5Oget_info_by_name(m_rootid, path.c_str(), &obj_info, H5P_DEFAULT, H5P_DEFAULT) < 0)
        return -ENOENT;
    return H5Ldelete(m_rootid, path.c_str(), H5P_DEFAULT);
}

herr_t
H5FSRoot::Mkgrp(const std::string&& path, mode_t mode){
    return (H5Gcreate(m_rootid, path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) >= 0) ? 0 : EACCES;
}
