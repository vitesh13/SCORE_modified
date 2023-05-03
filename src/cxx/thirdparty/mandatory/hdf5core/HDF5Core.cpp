/*
* This file is part of paraprobe-toolbox.
*
* paraprobe-toolbox is free software: you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation, either version 3 of the License,
*  or (at your option) any later version.
*
* paraprobe-toolbox is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with paraprobe-toolbox. If not, see <https://www.gnu.org/licenses/>.
*/

#include "HDF5Core.h"

io_info::io_info( vector<size_t> const & _shp )
{
	this->shape = vector<size_t>();
	this->chunk = vector<size_t>();
	this->compression = MYHDF5_COMPRESSION_NONE;
	this->compression_opts = 0x00;
	this->is_valid = true;
	if ( _shp.size() == 0 ) { //scalar when _shp.size() == 0, up to 4d array currently implemented
		this->shape = vector<size_t>();
	}
	else if ( 1 <= _shp.size() && _shp.size() <= 4 ) {
		this->shape = _shp;
	}
	else {
		this->is_valid = false;
	}
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
	cout << this << "\n";
#endif
}


io_info::io_info( vector<size_t> const & _shp, vector<size_t> const & _chk )
{
	this->shape = vector<size_t>();
	this->chunk = vector<size_t>();
	this->compression = MYHDF5_COMPRESSION_NONE;
	this->compression_opts = 0x00;
	this->is_valid = true;
	if ( _shp.size() == 0 ) { //scalar when _shp.size() == 0, up to 2d array currently implemented
		this->shape = vector<size_t>();
	}
	else if ( 1 <= _shp.size() && _shp.size() <= 4 ) {
		this->shape = _shp;
	}
	else {
		this->is_valid = false;
		return;
	}
	//compression only available with chunked data, otherwise uncompressed contiguous design as the fallback
	if ( _shp.size() > 0 && _chk.size() == _shp.size() ) {
		//no chunking for scalars and small vectors
		for( size_t i = 0; i < _shp.size(); i++ ) {
			//if ( _shp[i] % _chk[i] != 0 ) { //dimension size should be integer multiple of chunk dimension size
			//	this->chunk.push_back( _shp[i] );
			//}
			//else {
				this->chunk.push_back( _chk[i] );
			//}
		}
		if ( this->chunk.size() > 0 ) {
			this->compression = MYHDF5_COMPRESSION_GZIP;
			this->compression_opts = MYHDF5_COMPRESSION_DEFAULT;
		}
	}
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
	cout << this << "\n";
#endif
}


io_info::io_info( vector<size_t> const & _shp, vector<size_t> const & _chk,
			const unsigned char _cmp, const unsigned int _cmp_opts )
{
	this->shape = vector<size_t>();
	this->chunk = vector<size_t>();
	this->compression = MYHDF5_COMPRESSION_NONE;
	this->compression_opts = 0x00;
	this->is_valid = true;
	if ( _shp.size() == 0 ) { //scalar when _shp.size() == 0, up to 4d array currently implemented
		this->shape = vector<size_t>();
	}
	else if ( 1 <= _shp.size() && _shp.size() <= 4 ) {
		this->shape = _shp;
	}
	else {
		this->is_valid = false;
		return;
	}
	//compression only available with chunked data,
	//uncompressed contiguous design as the fallback
	//no chunking for scalars and small vectors
	if ( _shp.size() > 0 && _chk.size() == _shp.size() ) {
		for( size_t i = 0; i < _shp.size(); i++ ) {
			//if ( _shp[i] % _chk[i] != 0 ) { //dimension size should be integer multiple of chunk dimension size
			//	this->chunk.push_back( _shp[i] );
			//}
			//else {
				this->chunk.push_back( _chk[i] );
			//}
		}
		if ( _cmp == MYHDF5_COMPRESSION_GZIP ) {
			this->compression = MYHDF5_COMPRESSION_GZIP;
		}
		if ( _cmp_opts <= 9 ) {
			vector<unsigned char> available_strength = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
			unsigned char desired_strength = (unsigned char) _cmp_opts;
			for( auto it = available_strength.begin(); it != available_strength.end(); it++ ) {
				if ( desired_strength == *it ) {
					this->compression_opts = *it;
					break;
				}
			}
		}
		else {
			this->compression_opts = 0x00;
		}
	}
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
	cout << this << "\n";
#endif
}


ostream& operator << (ostream& in, io_info const & val)
{
	in << "io_info" << "\n";
	in << "shape.size() " << val.shape.size() << "\n";
	in << "chunk.size() " << val.chunk.size() << "\n";
	if ( val.compression == MYHDF5_COMPRESSION_GZIP )
		in << "compression " << "deflate" << "\n";
	else
		in << "compression " << "none" << "\n";
	in << "compression_opts " << (int) val.compression_opts << "\n";
	in << "is_valid " << (int) val.is_valid << "\n";
	return in;
}


ioAttributes::ioAttributes()
{
	u8 = map<string, unsigned char>();
	i8 = map<string, char>();
	u16 = map<string, unsigned short>();
	i16 = map<string, short>();
	u32 = map<string, unsigned int>();
	i32 = map<string, int>();
	u64 = map<string, unsigned long>();
	i64 = map<string, long>();
	f32 = map<string, float>();
	f64 = map<string, double>();
	str = map<string, string>();
	str_arr = map<string, vector<string>>();
	exists = map<string, bool>();
}


ioAttributes::~ioAttributes()
{
}


template<typename T> void ioAttributes::add( const string keyword, const T value )
{
	map<string, bool>::iterator thisone = exists.find( keyword );
	if ( thisone == exists.end() ) { //in HDF5, attribute keyword names have to be unique for an object
		if constexpr(std::is_same<T, unsigned char>::value) {
			u8[keyword] = value; exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "added attribute u08 " << keyword << " " << (int) value << "\n";
#endif
		}
		else if constexpr(std::is_same<T, char>::value) {
			i8[keyword] = value; exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "added attribute i08 " << keyword << " " << (int) value << "\n";
#endif
		}
		else if constexpr(std::is_same<T, unsigned short>::value) {
			u16[keyword] = value; exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "added attribute u16 " << keyword << " " << value << "\n";
#endif
		}
		else if constexpr(std::is_same<T, short>::value) {
			i16[keyword] = value; exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "added attribute i16 " << keyword << " " << value << "\n";
#endif
		}
		else if constexpr(std::is_same<T, unsigned int>::value) {
			u32[keyword] = value; exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "added attribute u32 " << keyword << " " << value << "\n";
#endif
		}
		else if constexpr(std::is_same<T, int>::value) {
			i32[keyword] = value; exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "added attribute i32 " << keyword << " " << value << "\n";
#endif
		}
		else if constexpr(std::is_same<T, unsigned long>::value) {
			u64[keyword] = value; exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "added attribute u64 " << keyword << " " << value << "\n";
#endif
		}
		else if constexpr(std::is_same<T, long>::value) {
			i64[keyword] = value; exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "added attribute i64 " << keyword << " " << value << "\n";
#endif
		}
		else if constexpr(std::is_same<T, float>::value) {
			f32[keyword] = value; exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "added attribute f32 " << keyword << " " << value << "\n";
#endif
		}
		else if constexpr(std::is_same<T, double>::value) {
			f64[keyword] = value; exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "added attribute f64 " << keyword << " " << value << "\n";
#endif
		}
		else {
			return;
		}
	}
}

template void ioAttributes::add( const string keyword, const unsigned char value );
template void ioAttributes::add( const string keyword, const char value );
template void ioAttributes::add( const string keyword, const unsigned short value );
template void ioAttributes::add( const string keyword, const short value );
template void ioAttributes::add( const string keyword, const unsigned int value );
template void ioAttributes::add( const string keyword, const int value );
template void ioAttributes::add( const string keyword, const unsigned long value );
template void ioAttributes::add( const string keyword, const long value );
template void ioAttributes::add( const string keyword, const float value );
template void ioAttributes::add( const string keyword, const double value );


void ioAttributes::add( const string keyword, const string value )
{
	map<string, bool>::iterator thisone = exists.find( keyword );
	if ( thisone == exists.end() ) { //in HDF5, attribute keyword names have to be unique for an object
		str[keyword] = value; exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "added attribute str " << keyword << " " << value << "\n";
#endif
	}
}


void ioAttributes::add( const string keyword, vector<string> & values )
{
	map<string, bool>::iterator thisone = exists.find( keyword );
	if ( thisone == exists.end() ) { //in HDF5, attribute keyword names have to be unique for an object
		str_arr[keyword] = vector<string>();
		for( auto kt = values.begin(); kt != values.end(); kt++ ) {
			str_arr[keyword].push_back( *kt );
		}
		exists[keyword] = true;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "added attribute str_arr " << keyword << " " << values.size() << "\n";
#endif
	}
}

//##MK::this will not work as intended due to rvalue issues and compiler cannot deduce the rvalue type

template<typename T> T ioAttributes::query( const string keyword )
{
	map<string, bool>::iterator thisone = exists.find( keyword );
	if ( thisone != exists.end() ) {
		if constexpr(std::is_same<T, unsigned char>::value) {
			map<string, unsigned char>::iterator jt = u8.find( keyword );
			if ( jt != u8.end() )
				return (unsigned char) jt->second;
		}
		else if constexpr(std::is_same<T, char>::value) {
			map<string, char>::iterator jt = i8.find( keyword );
			if ( jt != i8.end() )
				return (char) jt->second;
		}
		else if constexpr(std::is_same<T, unsigned short>::value) {
			map<string, unsigned short>::iterator jt = u16.find( keyword );
			if ( jt != u16.end() )
				return (unsigned short) jt->second;
		}
		else if constexpr(std::is_same<T, short>::value) {
			map<string, short>::iterator jt = i16.find( keyword );
			if ( jt != i16.end() )
				return (short) jt->second;
		}
		else if constexpr(std::is_same<T, unsigned int>::value) {
			map<string, unsigned int>::iterator jt = u32.find( keyword );
			if ( jt != u32.end() )
				return (unsigned int) jt->second;
		}
		else if constexpr(std::is_same<T, int>::value) {
			map<string, int>::iterator jt = i32.find( keyword );
			if ( jt != i32.end() )
				return (int) jt->second;
		}
		else if constexpr(std::is_same<T, unsigned long>::value) {
			map<string, unsigned long>::iterator jt = u64.find( keyword );
			if ( jt != u64.end() )
				return (unsigned long) jt->second;
		}
		else if constexpr(std::is_same<T, long>::value) {
			map<string, long>::iterator jt = i64.find( keyword );
			if ( jt != i64.end() )
				return (long) jt->second;
		}
		else if constexpr(std::is_same<T, float>::value) {
			map<string, float>::iterator jt = f32.find( keyword );
			if ( jt != f32.end() )
				return (float) jt->second;
		}
		else if constexpr(std::is_same<T, double>::value) {
			map<string, double>::iterator jt = f64.find( keyword );
			if ( jt != f64.end() )
				return (double) jt->second;
		}
		else {
			return (T) 0;
		}
		return (T) 0;
	}
	return (T) 0;
}

template unsigned char ioAttributes::query( const string keyword );
template char ioAttributes::query( const string keyword );
template unsigned short ioAttributes::query( const string keyword );
template short ioAttributes::query( const string keyword );
template unsigned int ioAttributes::query( const string keyword );
template int ioAttributes::query( const string keyword );
template unsigned long ioAttributes::query( const string keyword );
template long ioAttributes::query( const string keyword );
template float ioAttributes::query( const string keyword );
template double ioAttributes::query( const string keyword );


string ioAttributes::query( const string keyword )
{
	map<string, string>::iterator jt = str.find( keyword );
	if ( jt != str.end() ) {
		return jt->second;
	}
	return "";
}


void ioAttributes::report()
{
	cout << "Named attributes in the dictionary" << "\n";
	for( auto it = exists.begin(); it != exists.end(); it++ ) {
		if ( it->second == true ) {
			cout << it->first << "\n";
		}
	}
	cout << "Named u08 attributes in the dictionary" << "\n";
	for( auto it = u8.begin(); it != u8.end(); it++ ) {
		cout << it->first << ", " << (int) it->second << "\n";
	}
	cout << "Named i08 attributes in the dictionary" << "\n";
	for( auto it = i8.begin(); it != i8.end(); it++ ) {
		cout << it->first << ", " << (int) it->second << "\n";
	}
	cout << "Named u16 attributes in the dictionary" << "\n";
	for( auto it = u16.begin(); it != u16.end(); it++ ) {
		cout << it->first << ", " << it->second << "\n";
	}
	cout << "Named i16 attributes in the dictionary" << "\n";
	for( auto it = i16.begin(); it != i16.end(); it++ ) {
		cout << it->first << ", " << it->second << "\n";
	}
	cout << "Named u32 attributes in the dictionary" << "\n";
	for( auto it = u32.begin(); it != u32.end(); it++ ) {
		cout << it->first << ", " << it->second << "\n";
	}
	cout << "Named i32 attributes in the dictionary" << "\n";
	for( auto it = i32.begin(); it != i32.end(); it++ ) {
		cout << it->first << ", " << it->second << "\n";
	}
	cout << "Named u64 attributes in the dictionary" << "\n";
	for( auto it = u64.begin(); it != u64.end(); it++ ) {
		cout << it->first << ", " << it->second << "\n";
	}
	cout << "Named i64 attributes in the dictionary" << "\n";
	for( auto it = i64.begin(); it != i64.end(); it++ ) {
		cout << it->first << ", " << it->second << "\n";
	}
	cout << "Named f32 attributes in the dictionary" << "\n";
	for( auto it = f32.begin(); it != f32.end(); it++ ) {
		cout << it->first << ", " << it->second << "\n";
	}
	cout << "Named f64 attributes in the dictionary" << "\n";
	for( auto it = f64.begin(); it != f64.end(); it++ ) {
		cout << it->first << ", " << it->second << "\n";
	}
	cout << "Named string attributes in the dictionary" << "\n";
	for( auto it = str.begin(); it != str.end(); it++ ) {
		cout << it->first << ", " << it->second << "\n";
	}
}


HdfFiveSeqHdl::HdfFiveSeqHdl()
{
	h5resultsfn = "PARAPROBE.Test.nxs";
	fileid = 0; //H5I_INVALID_HID;
	dspcid = 0; //H5I_INVALID_HID;
	plistid = 0; //H5I_INVALID_HID;
	dsetid = 0; //H5I_INVALID_HID;
	//attr_buf = ioAttributes();
}


HdfFiveSeqHdl::HdfFiveSeqHdl( const string h5fn )
{
	h5resultsfn = h5fn;
	fileid = 0; //H5I_INVALID_HID;
	dspcid = 0; //H5I_INVALID_HID;
	plistid = 0; //H5I_INVALID_HID;
	dsetid = 0; //H5I_INVALID_HID;
	//attr_buf = ioAttributes();
}


HdfFiveSeqHdl::~HdfFiveSeqHdl()
{
}


//low-level functions which must not be called alone but only from within a function
int HdfFiveSeqHdl::nexus_open( const unsigned flags )
{
	//open an existent file in a mode given by flags
	fileid = H5Fopen( h5resultsfn.c_str(), flags, H5P_DEFAULT);
	if ( fileid != H5I_INVALID_HID ) {
		//nop
	}
	else {
		cerr << "Opening " << h5resultsfn << " failed !" << "\n";
	}
	return fileid;
}


int HdfFiveSeqHdl::nexus_close( const int previous_status )
{
	//low-level function to release dangling object handles correctly to not leave the handle manager polluted after an access to an object
	int current_status = previous_status;
	//if ( dspc == true ) {
	htri_t state = 0;
	state = H5Iis_valid(dspcid);
	if ( state > 0 ) { //dspcid is still valid and thus should be closed
		if ( H5Sclose( dspcid ) >= 0 ) {
			//nop
		}
		else {
			cerr << "H5Sclose error on " << h5resultsfn << "\n";
			current_status = MYHDF5_DSPCCLOSE_FAILED;
		}
	}
	else if ( state == 0 ) { //dspcid is no longer valid or call was errorneuous, dspcid must not be closed again
		//nop
	}
	else {
		cerr << "H5Iis_valid error on " << h5resultsfn << "\n";
		current_status = MYHDF5_FAILED;
	}

	state = H5Iis_valid(plistid);
	if ( state > 0 ) {
		if ( H5Pclose( plistid ) >= 0 ) {
			//nop
		}
		else {
			cerr << "H5Pclose error on " << h5resultsfn << "\n";
			current_status = MYHDF5_PLSTCLOSE_FAILED;
		}
	}
	else if ( state == 0 ) {
		//nop
	}
	else {
		cerr << "H5Iis_valid error on " << h5resultsfn << "\n";
		current_status = MYHDF5_FAILED;
	}

	state = H5Iis_valid(dsetid);
	if ( state > 0 ) {
		if ( H5Dclose( dsetid ) >= 0 ) {
			//nop
		}
		else {
			cerr << "H5Dclose error on " << h5resultsfn << "\n";
			current_status = MYHDF5_DSETCLOSE_FAILED;
		}
	}
	else if ( state == 0 ) {
		//nop
	}
	else {
		cerr << "H5Iis_valid error on " << h5resultsfn << "\n";
		current_status = MYHDF5_FAILED;
	}

	state = H5Iis_valid(fileid);
	if ( state > 0 ) {
		if ( H5Fclose( fileid ) >= 0 ) {
			//nop
		}
		else {
			cerr << "H5Fclose error on " << h5resultsfn << "\n";
			current_status = MYHDF5_FCLOSE_FAILED;
		}
	}
	else if ( state == 0 ) {
		//nop
	}
	else {
		cerr << "H5Iis_valid error on " << h5resultsfn << "\n";
		current_status = MYHDF5_FAILED;
	}

	return current_status;
}


//high-level function
int HdfFiveSeqHdl::nexus_create()
{
	 //create an HDF5 file truncating ALL eventually existent content when a file with the same name already exists
	fileid = H5Fcreate( h5resultsfn.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
	if ( fileid != H5I_INVALID_HID ) {
		if ( H5Fclose(fileid) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
			cout << h5resultsfn << " created successfully" << "\n";
#endif
			return MYHDF5_SUCCESS;
		}
		else {
			cerr << "H5Fclose failed on " << h5resultsfn << "\n";
			return MYHDF5_FCLOSE_FAILED;
		}
	}
	//dont use an invalid fileid to attempt closing the file
	return MYHDF5_FILEACCESS_FAILED;
}


bool HdfFiveSeqHdl::nexus_group_exists( const string h5_absolute_path )
{
	//check if a given path in an HDF5 file exists
	//mainly relevant to recursively instantiate a chain of nested groups to create a dataset deep down in the hierarchy
	if ( nexus_open( H5F_ACC_RDONLY ) == H5I_INVALID_HID ) {
		cerr << "Opening file during link exists returned invalid fileid!" << "\n";
		return false;
	}
	//now the file is open otherwise we wouldnt have gotten here

	//check if the path in the open file pointed to by fileid exists
	//cout << "absolute_path __" << h5_absolute_path << "__" << "\n";
	vector<string> level_by_level = split_absolute_path( h5_absolute_path );

	//start at the file, root
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
	cout << "Checking if each group exists level-by-level..." << "\n";
#endif
	hid_t curr_loc_id = fileid;
	string curr_group = "";
	string current_path = "";
	string absolute_path = "/" + h5_absolute_path;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
	cout << "absolute_path __" << absolute_path << "__" << "\n";
#endif

	//##MK::https://gist.github.com/jzrake/3025642
	for( size_t i = 0; i < level_by_level.size(); i++ ) {
		curr_group = level_by_level[i];
		current_path += "/" + level_by_level[i];
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "Currently visiting " << curr_group << "\n";
		cout << "current_path __" << current_path << "__" << "\n";
#endif
		htri_t status = H5Lexists( curr_loc_id, curr_group.c_str(), H5P_DEFAULT );
		//htri_t three-valued outcome, exists (>0), does not exist (==0), error (<0)
		if( status > 0 ) { //exists
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << curr_group << " --> exists" << "\n";
#endif
			//only really open the group if we have not yet arrived at location named h5_absolute_path
			//take two examples
			///entry/group, this case will work
			///entry/dset, this case will throw because yes /entry/dset may exists as reported by H5Lexists but H5Gopen cannot be applied to it dset because it is
			if ( current_path.compare( absolute_path ) != 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "Not yet arrived at absolute path" << "\n";
#endif
				hid_t next_loc_id = H5Gopen( curr_loc_id, curr_group.c_str(), H5P_DEFAULT );
				if ( next_loc_id == H5I_INVALID_HID ) { //cant traverse further, need to close file...
					if ( H5Fclose( fileid ) >= 0 ) {
						cerr << "Successful H5Fclose after failed H5Gopen on " << curr_group << "\n";
						return false;
					}
					cerr << "Failed H5Fclose after failed H5Gopen on " << curr_group << "\n";
					return false;
				}
				else { //traverse further
					if ( curr_loc_id != fileid ) {
						if ( H5Gclose( curr_loc_id ) >= 0 ) {
							//nop
						}
						else {
							if ( H5Fclose( fileid ) >= 0 ) {
								cerr << "Successful H5Fclose after a failed H5Gclose while preparing walk after " << curr_group << "\n";
								return  false;
							}
							cerr << "Failed H5Fclose after a failed H5Gclose while preparing walk after " << curr_group << "\n";
							return false;
						}
					}
					curr_loc_id = next_loc_id;
					continue;
				}
			} //not yet arrived at the target absolute path
			else {
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "Arrived at absolute path, stopping traversal" << "\n";
#endif
				if ( H5Fclose( fileid ) >= 0 ) {
					return true;
				}
				cerr << "Failed H5Fclose after a successful H5Gclose while stopping traversal " << curr_group << "\n";
				return false;
			}
		}
		else { //status <= 0
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << curr_group << " --> does not exist" << "\n";
#endif
			if ( curr_loc_id != fileid ) {
				if ( H5Gclose( curr_loc_id ) >= 0 ) {
					if ( H5Fclose( fileid ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
						cout << "Successful H5Fclose after a successful H5Gclose when link did not exist " << curr_group << "\n";
#endif
						return false;
					}
					cerr << "Failed H5Fclose after a successful H5Gclose when link did not exist " << curr_group << "\n";
					return false;
				}
				else {
					if ( H5Fclose( fileid ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
						cout << "Successful H5Fclose after a failed H5Gclose when link did not exist " << curr_group << "\n";
#endif
						return false;
					}
					cerr << "Failed H5Fclose after a failed H5Gclose when link did not exist " << curr_group << "\n";
					return false;
				}
			}
			else { //no need to close a group because we are still at fileid, i.e. root
				if ( H5Fclose( fileid ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
					cout << "Successful H5Fclose when link did not exist " << curr_group << "\n";
#endif
					return false;
				}
				cerr << "Failed H5Fclose when link did not exist " << curr_group << "\n";
				return false;
			}
		}
	}

	//success is achieved only when you have not previous run into a failure
	//such as having to stop the walk because of a missing path
	if ( H5Gclose( curr_loc_id ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
		cout << "Successful H5Gclose during cleanup" << "\n";
#endif
		if ( H5Fclose( fileid) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
			cout << "Successful H5Fclose during cleanup" << "\n";
#endif
			return true;
		}
		else {
			cerr << "Failed H5Fclose during cleanup" << "\n";
			return false;
		}
	}
	else {
		cerr << "Failed H5Gclose during cleanup" << "\n";
		if ( H5Fclose( fileid ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
			cout << "Successful H5Fclose after failed H5Gclose during cleanup" << "\n";
#endif
			return false;
		}
		else {
			cerr << "Failed H5Fclose after failed H5Gclose during cleanup" << "\n";
			return false;
		}
	}
}


int HdfFiveSeqHdl::nexus_read_attributes( const string dsnm, ioAttributes & attrs )
{
	//MK::this function must only be called from within a nexus_read template instance as it
	//assumes/reuses open(ed) resources on an existent file and dataset to reduce costs
	//for checks and unnecessary interaction with the HDF5 library !

	//assume fileid, dset are set to valid resources on an existent dataset entry dsnm in an open file
	//this function extracts all scalar attributes for elementary types
	//https://fossies.org/linux/hdf5/examples/h5_attribute.c

	//clear the class-internal attribute buffer
	//attr_buf = ioAttributes();
	attrs = ioAttributes();

	if ( nexus_open( H5F_ACC_RDONLY ) != H5I_INVALID_HID ) {
		//if ( nexus_dataset_exists( dsnm ) == true ) { //H5Gget_objinfo( fileid, dsnm.c_str(), 0, NULL) ) >= 0 ) {
			dsetid = H5Dopen2( fileid, dsnm.c_str(), H5P_DEFAULT );
			if ( dsetid != H5I_INVALID_HID ) {
				H5O_info2_t oinfo;
				if ( H5Oget_info3(dsetid, &oinfo, H5O_INFO_NUM_ATTRS) >= 0 ) {

					for ( unsigned i = 0; i < (unsigned) oinfo.num_attrs; i++ ) {
						hid_t attrid = H5Aopen_by_idx( dsetid, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, (hsize_t) i, H5P_DEFAULT, H5P_DEFAULT );
						if ( attrid >= 0 ) {
							hid_t dtype = H5Aget_type(attrid);
							if ( dtype != H5I_INVALID_HID ) {
								//parse details of the attribute to identify which datatype was used
								char* rbuf = new char[U16MX];
								ssize_t nbytes = H5Aget_name(attrid, (size_t) U16MX, rbuf );
								H5T_class_t dtype_class_id = H5Tget_class(dtype);
								size_t dtype_size = H5Tget_size(dtype);
								H5T_sign_t dtype_sign = H5Tget_sign(dtype);

								stringstream linestream;
								for( ssize_t j = 0; j < nbytes; j++  ) {
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
									cout << "-->__" << rbuf[j] << "__" << "\n";
#endif
									linestream << rbuf[j];
								}
								delete [] rbuf; rbuf = NULL;
								string keyword = linestream.str();

#ifdef MYHDF5_VERBOSE_LEVEL_TWO
								cout << "attribute name " << keyword << "\n";
								cout << "attribute dtype class id " << dtype_class_id << "\n";
								cout << "attribute dtype class size bytes " << dtype_size << "\n";
								cout << "attribute dtype sgn " << dtype_sign << "\n";
#endif

								//extract interpretable scalar or string attribute values of specific type
								//assuming that paraprobe uses the H5T specific little endian variants
								switch(dtype_class_id)
								{
									case H5T_INTEGER:
									{
										switch(dtype_size)
										{
											case 1:
											{
												if ( dtype_sign == H5T_SGN_NONE ) { //H5T_STD_U8LE
													unsigned char value = 0x00;
													if ( H5Aread(attrid, dtype, &value ) >= 0 ) {
														attrs.add( keyword, value );
													}
												}
												else if ( dtype_sign == H5T_SGN_2 ) { //H5T_STD_I8LE
													char value = 0x00;
													if ( H5Aread(attrid, dtype, &value ) >= 0 ) {
														attrs.add( keyword, value );
													}
												}
												else {
													//H5T_SGN_ERROR
												}
												break;
											}
											case 2:
											{
												if ( dtype_sign == H5T_SGN_NONE ) { //H5T_STD_U16LE
													unsigned short value = 0;
													if ( H5Aread(attrid, dtype, &value ) >= 0 ) {
														attrs.add( keyword, value );
													}
												}
												else if ( dtype_sign == H5T_SGN_2 ) { //H5T_STD_I16LE
													short value = 0;
													if ( H5Aread(attrid, dtype, &value ) >= 0 ) {
														attrs.add( keyword, value );
													}
												}
												else {
													//H5T_SGN_ERROR
												}
												break;
											}
											case 4:
											{
												if ( dtype_sign == H5T_SGN_NONE ) { //H5T_STD_U32LE
													unsigned int value = 0;
													if ( H5Aread(attrid, dtype, &value ) >= 0 ) {
														attrs.add( keyword, value );
													}
												}
												else if ( dtype_sign == H5T_SGN_2 ) { //H5T_STD_I32LE
													int value = 0;
													if ( H5Aread(attrid, dtype, &value ) >= 0 ) {
														attrs.add( keyword, value );
													}
												}
												else {
													//H5T_SGN_ERROR
												}
												break;
											}
											case 8:
											{
												if ( dtype_sign == H5T_SGN_NONE ) { //H5T_STD_U64LE
													unsigned long value = 0;
													if ( H5Aread(attrid, dtype, &value ) >= 0 ) {
														attrs.add( keyword, value );
													}
												}
												else if ( dtype_sign == H5T_SGN_2 ) { //H5T_STD_I64LE
													long value = 0;
													if ( H5Aread(attrid, dtype, &value ) >= 0 ) {
														attrs.add( keyword, value );
													}
												}
												else {
													//H5T_SGN_ERROR
												}
												break;
											}
											default:
											{
												break;
											}
										}
										break;
									}
									case H5T_FLOAT:
									{
										if ( dtype_size == 4 ) {
											float value = MYZERO;
											if ( H5Aread(attrid, dtype, &value ) >= 0 ) {
												attrs.add( keyword, value );
											}
										}
										else if ( dtype_size == 8 ) {
											double value = MYZERO;
											if ( H5Aread(attrid, dtype, &value ) >= 0 ) {
												attrs.add( keyword, value );
											}
										}
										else {
											//nope
										}
										break;
									}
									case H5T_STRING:
									{
										hid_t atype_mem = H5Tget_native_type(dtype, H5T_DIR_ASCEND);
										if ( atype_mem != H5I_INVALID_HID ) {
											char* rbuf = new char[U16MX];
											if ( H5Aread(attrid, atype_mem, rbuf ) >= 0 ) {
												stringstream tmpstream;
												for( size_t j = 0; j < dtype_size; j++  ) {
													if ( rbuf[j] != 0x00 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
														cout << "------>__" << rbuf[j] << "__" << "\n";
#endif
														tmpstream << rbuf[j];
													}
													else {
														break;
													}
												}
												string value = tmpstream.str();
												delete [] rbuf; rbuf = NULL;
												if ( value.length() > 0 ) {
													attrs.add( keyword, value );
												}
											}
											delete [] rbuf; rbuf = NULL;
										}
										break;
									}
									default:
									{
										break;
									}
								}
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
								cout << "Reading " << dsnm << " attribute keyword " << keyword << " success" << "\n";
#endif
							}
							else {
								if ( H5Aclose(attrid) >= 0 ) {
									cerr << "Reading " << dsnm << " attribute failed, dtype invalid will continue with next attribute !" << "\n";
									continue;
								}
								else {
									cerr << "Reading " << dsnm << " attribute failed, unable to close opened attribute, stopping reading further attributes !" << "\n";
									break;
								}
							}
						}
						else {
							cerr << "Reading " << dsnm << " attribute failed, attribute not openable, stopping reading further attributes !" << "\n";
							break;
						}
					} //inspect the next attribute
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
					cout << "Reading " << dsnm << " all attributes visited" << "\n";
#endif
				}
				if ( H5Dclose( dsetid ) >= 0 ) {
					if ( H5Fclose( fileid ) >= 0 ) {
						return MYHDF5_SUCCESS;
					}
					cerr << "H5Fclose error on " << h5resultsfn << "\n";
					return MYHDF5_FCLOSE_FAILED;
				}
				cerr << "H5Dclose error on " << h5resultsfn << "\n";
				if ( H5Fclose( fileid ) >= 0 ) {
					return MYHDF5_DSETCLOSE_FAILED;
				}
				cerr << "H5Fclose error on " << h5resultsfn << "\n";
				return MYHDF5_FCLOSE_FAILED;
			}
			cerr << "H5Dopen error on " << h5resultsfn << "\n";
			if ( H5Fclose( fileid ) >= 0 ) {
				return MYHDF5_DSETOPEN_FAILED;
			}
			cerr << "H5Fclose error on " << h5resultsfn << "\n";
			return MYHDF5_FCLOSE_FAILED;
		//}
	}
	return MYHDF5_NOT_EXPECTED;

	/*
	// get attribute info using iteration function
	if ( H5Aiterate2(dsetid, H5_INDEX_NAME, H5_ITER_INC, NULL, attr_info, NULL) >= 0 ) {
		//copy over the attributes from the buffer and clear that buffer
		attrs = attr_buf;
		attr_buf = ioAttributes();
		return MYHDF5_SUCCESS;
	}
	return MYHDF5_ATTRREAD_FAILED;
	*/
}


template<typename T> int HdfFiveSeqHdl::nexus_read( const string dsnm, T & retval )
{
	vector<T> tmp = vector<T>();
	if ( nexus_read( dsnm, tmp ) == MYHDF5_SUCCESS ) {
		if ( tmp.size() >= 1 ) {
			retval = tmp[0];
			return MYHDF5_SUCCESS;
		}
		cerr << "Reading " << dsnm << " scalar, failed, tmp.size() == 0 !" << "\n";
		return MYHDF5_NOT_EXPECTED;
	}
	cerr << "Reading " << dsnm << " scalar, failed, output left unchanged !" << "\n";
	return MYHDF5_DSETREAD_FAILED;
}

template int HdfFiveSeqHdl::nexus_read( const string dsnm, unsigned char & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, char & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, unsigned short & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, short & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, unsigned int & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, int & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, unsigned long & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, long & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, float & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, double & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, string & retval );


template<typename T> int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<T> & retval )
{
	//open file in read-only mode to not modified the file access timestamps
	hid_t dtype = 0; //H5T_STD_U32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
	cout << "dtype " << dtype << "\n";
#endif
	size_t max_characters = 0;
	if constexpr(std::is_same<T, unsigned char>::value) {
		dtype = H5T_STD_U8LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype u8 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, char>::value) {
		dtype = H5T_STD_I8LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype i8 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, unsigned short>::value) {
		dtype = H5T_STD_U16LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype u16 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, short>::value) {
		dtype = H5T_STD_I16LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype i16 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, unsigned int>::value) {
		dtype = H5T_STD_U32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype u32 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, int>::value) {
		dtype = H5T_STD_I32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype i32 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, unsigned long>::value) {
		dtype = H5T_STD_U64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype u64 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, long>::value) {
		dtype = H5T_STD_I64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype i64 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, float>::value) {
		dtype = H5T_IEEE_F32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype f32 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, double>::value) {
		dtype = H5T_IEEE_F64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype f64 " << dtype << "\n";
#endif
	}
	else {
		//nop
	}

	retval = vector<T>();
	if ( nexus_open( H5F_ACC_RDONLY ) != H5I_INVALID_HID ) {
		//if ( nexus_dataset_exists( dsnm ) == true ) { //H5Gget_objinfo( fileid, dsnm.c_str(), 0, NULL) ) >= 0 ) {
			dsetid = H5Dopen2( fileid, dsnm.c_str(), H5P_DEFAULT );
			if ( dsetid != H5I_INVALID_HID ) {

				if constexpr(std::is_same<T, string>::value) {
					dtype = H5Dget_type( dsetid );
					if ( dtype != H5I_INVALID_HID ) {
						max_characters = H5Tget_size(dtype);
						//max_characters++; //null terminator
					}
					else {
						cerr << "Reading " << dsnm << " failed, get type !" << "\n";
						return MYHDF5_NOT_EXPECTED;
					}
				}

				plistid = H5Dget_create_plist( dsetid );
				if ( plistid != H5I_INVALID_HID ) {
					unsigned int flags;
					unsigned filter_info;
					H5Z_filter_t filter_type;
					int n_filters = H5Pget_nfilters(plistid);
					for ( int i = 0; i < n_filters; i++) {
						size_t n_elements = 0;
						filter_type = H5Pget_filter2( plistid, (unsigned) i, &flags, &n_elements, NULL, 0, NULL, &filter_info);
						if ( filter_type >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
							cout << "Detected the following filter: ";
							switch(filter_type)
							{
								case H5Z_FILTER_DEFLATE: { cout << "H5Z_FILTER_DEFLATE" << "\n"; break; }
								case H5Z_FILTER_SZIP: { cout << "H5Z_FILTER_SZIP" << "\n"; break; }
								default: { cout << "OTHER" << "\n"; break; }
							}
#endif
						}
						else {
							cerr << "Reading " << dsnm << " failed, reading filter !" << "\n";
							return nexus_close( MYHDF5_PLSTFILTER_FAILED ); //, true, true, true, true );
						}
					} //all filters inspected

					dspcid = H5Dget_space( dsetid );
					if ( dspcid != H5I_INVALID_HID ) {
						size_t n_values = 1;
						const int ndims = H5Sget_simple_extent_ndims( dspcid );
						if ( ndims == 0 ) {
							//nop, scalar
						}
						else if ( ndims == 1 ) {
							hsize_t dims[1] = { 0 };
							if ( H5Sget_simple_extent_dims(dspcid, dims, NULL) == ndims ) {
								n_values = dims[0];
							}
							else {
								return nexus_close( MYHDF5_DSETACCESS_FAILED );
							}
						}
						else if ( ndims == 2 ) {
							hsize_t dims[2] = { 0, 0 };
							if ( H5Sget_simple_extent_dims(dspcid, dims, NULL) == ndims ) {
								n_values = (size_t) dims[0] * (size_t) dims[1];
							}
							else {
								return nexus_close( MYHDF5_DSETACCESS_FAILED );
							}
						}
						else {
							cerr << "Reading " << dsnm << " failed, ndims is not 0, 1, or 2 but ndims is " << ndims << "\n";
							return nexus_close( MYHDF5_NOT_EXPECTED);
						}

						if constexpr(std::is_same<T, string>::value) {
							vector<char> tmp = vector<char>( n_values * max_characters, 0x00 );
							hid_t mspcid = H5Tcopy(H5T_C_S1);
							if ( mspcid >= 0 ) {
								if ( H5Tset_size( mspcid, max_characters ) >= 0 ) {
									if ( H5Dread( dsetid, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, tmp.data() ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
										cout << "Reading " << dsnm << " success" << "\n";
#endif
										/*
										for( auto jt = tmp.begin(); jt != tmp.end(); jt++ )
											cout << (int) *jt << "\n";
										*/
										retval.push_back( string( tmp.begin(), tmp.end() ) );
										if ( H5Tclose( mspcid ) >= 0 ) {
											return nexus_close( MYHDF5_SUCCESS );
										}
										cerr << "Reading " << dsnm << " failed, mspcid close !" << "\n";
										return MYHDF5_NOT_EXPECTED;
									}
									cerr << "Reading " << dsnm << " failed !" << "\n";
									if ( H5Tclose( mspcid ) >= 0 )
										return nexus_close( MYHDF5_NOT_EXPECTED );
									return nexus_close( MYHDF5_DSETREAD_FAILED );
								}
								cerr << "Reading " << dsnm << " failed, set size !" << "\n";
								if ( H5Tclose( mspcid ) >= 0 )
									return nexus_close( MYHDF5_NOT_EXPECTED );
								return nexus_close( MYHDF5_NOT_EXPECTED );
							}
						}
						else {
							retval = vector<T>( n_values, (T) 0 );
							if ( H5Dread( dsetid, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, retval.data() ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
								cout << "Reading " << dsnm << " success" << "\n";
#endif
								return nexus_close( MYHDF5_SUCCESS );
							}
						}

						cerr << "Reading " << dsnm << " failed, read !" << "\n";
						return nexus_close( MYHDF5_DSETREAD_FAILED );
					}
					cerr << "Reading " << dsnm << " failed, dspc !" << "\n";
					return nexus_close( MYHDF5_DSPCACCESS_FAILED );
				}
				cerr << "Reading " << dsnm << " failed, read plist !" << "\n";
				return nexus_close( MYHDF5_PLSTACCESS_FAILED );
			}
			cerr << "Reading " << dsnm << " failed, dset !" << "\n";
			return nexus_close( MYHDF5_DSETACCESS_FAILED );
		//}
		//cerr << "Reading " << dsnm << " failed, does not exists !" << "\n";
		//return nexus_close( MYHDF5_DSETACCESS_FAILED );
	}
	cerr << "Opening file " << h5resultsfn << " failed, output cleared !" << "\n";
	return MYHDF5_FOPEN_FAILED;
}

template int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<unsigned char> & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<char> & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<unsigned short> & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<short> & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<unsigned int> & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<int> & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<unsigned long> & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<long> & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<float> & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<double> & retval );
template int HdfFiveSeqHdl::nexus_read( const string dsnm, vector<string> & retval );


template<typename T> int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<T> & value )
{
	//MK::this function may only be called from nexus_write_attribute as it adds a typed and named attribute
	//to an (assumed existent) dataset referred to by dsetid inside an RDWR-opened HDF5 file referred to by fileid
	//for a specific type with the name in the keyword and the value in the value variable
	//see also an example for string arrays here https://svn.ssec.wisc.edu/repos/geoffc/C/HDF5/Examples_by_API/h5ex_t_stringatt.c
	if ( value.size() >= 1 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "nexus_write_attribute_value " << "_" << keyword << "_|_" << value.at(0) << "_" << "\n";
#endif
		hid_t attr_dtypid = 0;
		//size_t max_characters = 0;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "attr_dtypid " << attr_dtypid << "\n";
#endif
		if constexpr(std::is_same<T, unsigned char>::value) {
			attr_dtypid = H5T_STD_U8LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "attr_dtypid u8 " << attr_dtypid << "\n";
#endif
		}
		else if constexpr(std::is_same<T, char>::value) {
			attr_dtypid = H5T_STD_I8LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "attr_dtypid i8 " << attr_dtypid << "\n";
#endif
		}
		else if constexpr(std::is_same<T, unsigned short>::value) {
			attr_dtypid = H5T_STD_U16LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "attr_dtypid u16 " << attr_dtypid << "\n";
#endif
		}
		else if constexpr(std::is_same<T, short>::value) {
			attr_dtypid = H5T_STD_I16LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "attr_dtypid i16 " << attr_dtypid << "\n";
#endif
		}
		else if constexpr(std::is_same<T, unsigned int>::value) {
			attr_dtypid = H5T_STD_U32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "attr_dtypid u32 " << attr_dtypid << "\n";
#endif
		}
		else if constexpr(std::is_same<T, int>::value) {
			attr_dtypid = H5T_STD_I32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "attr_dtypid i32 " << attr_dtypid << "\n";
#endif
		}
		else if constexpr(std::is_same<T, unsigned long>::value) {
			attr_dtypid = H5T_STD_U64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "attr_dtypid u64 " << attr_dtypid << "\n";
#endif
		}
		else if constexpr(std::is_same<T, long>::value) {
			attr_dtypid = H5T_STD_I64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "attr_dtypid i64 " << attr_dtypid << "\n";
#endif
		}
		else if constexpr(std::is_same<T, float>::value) {
			attr_dtypid = H5T_IEEE_F32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "attr_dtypid f32 " << attr_dtypid << "\n";
#endif
		}
		else if constexpr(std::is_same<T, double>::value) {
			attr_dtypid = H5T_IEEE_F64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "attr_dtypid f64 " << attr_dtypid << "\n";
#endif
		}
		else if constexpr(std::is_same<T, string>::value) {
			size_t max_characters = 0;
			for( size_t k = 0; k < value.size(); k++ ) {
				if ( value.at(k).length() >= max_characters ) {
					max_characters = value[k].length(); // + 1; //include null terminator
				}
			}
			if ( max_characters == 0 ) {
				cerr << "max_characters must not be zero !" << "\n";
				return MYHDF5_NOT_EXPECTED;
			}
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "sdims " << max_characters << "\n";
#endif
			attr_dtypid = H5Tcopy(H5T_C_S1);
			if ( attr_dtypid != H5I_INVALID_HID ) {
				if ( H5Tset_size(attr_dtypid, max_characters) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
					cout << "attr_dtypid str " << attr_dtypid << "\n";
#endif
				}
				else {
					cerr << "attr_dtypid str set size " << attr_dtypid << "\n";
					return MYHDF5_NOT_EXPECTED;
				}
			}
			else {
				cerr << "attr_dtypid str c_s1 type " << attr_dtypid << "\n";
				return MYHDF5_NOT_EXPECTED;
			}
		}
		else {
			cerr << "attr_dtypid str " << attr_dtypid << "\n";
			return MYHDF5_NOT_IMPLEMENTED;
		}

		int rank = 1;
		hsize_t dims[1] = { 1 };  //always adding scalar values
		//hsize_t maxdims[1] = { 1 };
		if constexpr(std::is_same<T, string>::value) {
			if ( value.size() == 1 ) {
				dims[0] = 1;
				//maxdims[0] = 1;
			}
			else {
				dims[0] = (hsize_t) value.size();
				//maxdims[0] = NULL;
			}
		}
		hid_t aspcid = H5Screate_simple( rank, dims, NULL ); //##MK::##### maxdims );
		if ( aspcid != H5I_INVALID_HID ) {
			hid_t attrid = H5Acreate2(dsetid, keyword.c_str(), attr_dtypid, aspcid, H5P_DEFAULT, H5P_DEFAULT);
			if ( attrid >= 0 ) {
				if constexpr(std::is_same<T, string>::value) {
					//const char* wbuf[] = { value.c_str() };
					if ( value.size() == 1 ) {
						if ( H5Awrite(attrid, attr_dtypid, value.at(0).c_str() ) >= 0 ) {
							//##MK::if the attribute write failed
							//##it might happen that attrid and attr_dtypid are not closed properly
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
							cout << "Writing attribute " << keyword << " success" << "\n";
#endif
							if ( H5Aclose(attrid) >= 0 ) {
								if ( H5Sclose(aspcid) >= 0 ) {
									return MYHDF5_SUCCESS;
								}
								cerr << "H5Sclose on keyword " << keyword << " failed!" << "\n";
								return MYHDF5_ATTRCLOSE_FAILED;
							}
							cerr << "H5Aclose on keyword " << keyword << " failed!" << "\n";
							return MYHDF5_ATTRCLOSE_FAILED;
						}
						else {
							cerr << "Writing attribute " << keyword << " failed, write !" << "\n";
							if ( H5Aclose(attrid) >= 0 ) {
								if ( H5Sclose(aspcid) >= 0 ) {
									return MYHDF5_SUCCESS;
								}
								cerr << "H5Sclose on keyword " << keyword << " failed!" << "\n";
								return MYHDF5_ATTRCLOSE_FAILED;
							}
							cerr << "H5Aclose on keyword " << keyword << " failed!" << "\n";
							return MYHDF5_ATTRCLOSE_FAILED;
						}
					}
					else {
						/*
						char** wdata = new char*[value.size()];
						for( size_t k = 0; k < value.size(); k++ ) {
							wdata[k] = new char[max_characters];
						}
						*/
						if ( H5Awrite(attrid, attr_dtypid, value.data() ) >= 0 ) {
							//##MK::if the attribute write failed
							//##it might happen that attrid and attr_dtypid are not closed properly
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
							cout << "Writing attribute " << keyword << " success" << "\n";
#endif
							if ( H5Aclose(attrid) >= 0 ) {
								if ( H5Sclose(aspcid) >= 0 ) {
									return MYHDF5_SUCCESS;
								}
								cerr << "H5Sclose on keyword " << keyword << " failed!" << "\n";
								return MYHDF5_ATTRCLOSE_FAILED;
							}
							cerr << "H5Aclose on keyword " << keyword << " failed!" << "\n";
							return MYHDF5_ATTRCLOSE_FAILED;
						}
						else {
							cerr << "Writing attribute " << keyword << " failed, write !" << "\n";
							if ( H5Aclose(attrid) >= 0 ) {
								if ( H5Sclose(aspcid) >= 0 ) {
									return MYHDF5_SUCCESS;
								}
								cerr << "H5Sclose on keyword " << keyword << " failed!" << "\n";
								return MYHDF5_ATTRCLOSE_FAILED;
							}
							cerr << "H5Aclose on keyword " << keyword << " failed!" << "\n";
							return MYHDF5_ATTRCLOSE_FAILED;
						}
					}
				}
				else {
					T input_value = value.at(0);
					if ( H5Awrite(attrid, attr_dtypid, &input_value ) >= 0 ) { //##MK::&value
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
						cout << "Writing attribute " << keyword << " success" << "\n";
#endif
						if ( H5Aclose(attrid) >= 0 ) {
							if ( H5Sclose(aspcid) >= 0 ) {
								return MYHDF5_SUCCESS;
							}
							cerr << "H5Sclose on keyword " << keyword << " failed!" << "\n";
							return MYHDF5_ATTRCLOSE_FAILED;
						}
						cerr << "H5Aclose on keyword " << keyword << " failed!" << "\n";
						return MYHDF5_ATTRCLOSE_FAILED;
					}
					else {
						cerr << "Writing attribute " << keyword << " failed, write !" << "\n";
						if ( H5Aclose(attrid) >= 0 ) {
							if ( H5Sclose(aspcid) >= 0 ) {
								return MYHDF5_SUCCESS;
							}
							cerr << "H5Sclose on keyword " << keyword << " failed!" << "\n";
							return MYHDF5_ATTRCLOSE_FAILED;
						}
						cerr << "H5Aclose on keyword " << keyword << " failed!" << "\n";
						return MYHDF5_ATTRCLOSE_FAILED;
					}
				}
			}
			else {
				cerr << "Writing attribute " << keyword << " failed, create specific !" << "\n";
				if ( H5Sclose(aspcid) >= 0 ) {
					return MYHDF5_ATTRACCESS_FAILED;
				}
				cerr << "H5Sclose on keyword " << keyword << " failed!" << "\n";
				return MYHDF5_ATTRREAD_FAILED;
			}
		}
		cerr << "Writing attribute " << keyword << " failed, create simple !" << "\n";
		return MYHDF5_ATTRCREATE_FAILED;
	}
	cerr << "H5 core, error on input size while writing attribute keyword " << keyword << " failed!" << "\n";
	return MYHDF5_ERROR_INPUT_SIZE;
}


template int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<unsigned char> & value );
template int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<char> & value );
template int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<unsigned short> & value );
template int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<short> & value );
template int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<unsigned int> & value );
template int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<int> & value );
template int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<unsigned long> & value );
template int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<long> & value );
template int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<float> & value );
template int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<double> & value );
template int HdfFiveSeqHdl::nexus_write_attribute_value( const string keyword, vector<string> & value );


int HdfFiveSeqHdl::nexus_write_attributes( const string loc_name, ioAttributes const & attrs )
{
	//MK::this function must only be called from within a nexus_write template instance as it
	//assumes/reuses open(ed) resources on an existent file and dataset to reduce costs
	//for checks and unnecessary interaction with the HDF5 library !

	//assume fileid, dset are set to valid resources on an existent dataset entry loc_name in an open file
	htri_t status = 0;

	//<0 (failure), == 0 (false), == 1 (true)
	//https://docs.hdfgroup.org/hdf5/develop/_h5public_8h.html#aa8f6c28736dbd0f18388c67911d38aca
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
	cout << "Writing attributes to loc_name ..." << loc_name << "\n";
#endif

//scalar attributes
	for( auto it = attrs.u8.begin(); it != attrs.u8.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding u8 attribute " << it->first << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<unsigned char> value_input = { it->second };
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding u8 attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			} //do not return something because we want to add further attributes
			else {
				cerr << "Adding u8 attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//##MK::##################
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else { //status < 0
			cerr << "Adding u8 attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

	for( auto it = attrs.i8.begin(); it != attrs.i8.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding i8 attribute " << it->first << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<char> value_input = { it->second };
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding i8 attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			}
			else {
				cerr << "Adding i8 attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//##MK::##################
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else {
			cerr << "Adding i8 attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

	for( auto it = attrs.u16.begin(); it != attrs.u16.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding u16 attribute " << it->first << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<unsigned short> value_input = { it->second };
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding u16 attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			}
			else {
				cerr << "Adding u16 attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//##MK::##################
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else {
			cerr << "Adding u16 attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

	for( auto it = attrs.i16.begin(); it != attrs.i16.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding i16 attribute " << it->first << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<short> value_input = { it->second };
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding i16 attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			}
			else {
				cerr << "Adding i16 attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//##MK::###########
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else {
			cerr << "Adding i16 attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

	for( auto it = attrs.u32.begin(); it != attrs.u32.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding u32 attribute " << it->first << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<unsigned int> value_input = { it->second };
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding u32 attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			}
			else {
				cerr << "Adding u32 attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//##MK::###############
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else {
			cerr << "Adding u32 attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

	for( auto it = attrs.i32.begin(); it != attrs.i32.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding i32 attribute " << it->first << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<int> value_input = { it->second };
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding i32 attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			}
			else {
				cerr << "Adding i32 attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//##MK::###############
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else {
			cerr << "Adding i32 attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

	for( auto it = attrs.u64.begin(); it != attrs.u64.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding u64 attribute " << it->first << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<unsigned long> value_input = { it->second };
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding u64 attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			}
			else {
				cerr << "Adding u64 attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//##MK::####
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else {
			cerr << "Adding u64 attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

	for( auto it = attrs.i64.begin(); it != attrs.i64.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding i64 attribute " << it->first << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<long> value_input = { it->second };
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding i64 attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			}
			else {
				cerr << "Adding i64 attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//##MK::######
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else {
			cerr << "Adding i64 attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

	for( auto it = attrs.f32.begin(); it != attrs.f32.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding f32 attribute " << it->first << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<float> value_input = { it->second };
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding f32 attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			}
			else {
				cerr << "Adding f32 attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//##MK::######
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else {
			cerr << "Adding f32 attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

	for( auto it = attrs.f64.begin(); it != attrs.f64.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding f64 attribute " << it->first << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<double> value_input = { it->second };
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding f64 attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			}
			else {
				cerr << "Adding f64 attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//##MK::################
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else {
			cerr << "Adding f64 attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

	for( auto it = attrs.str.begin(); it != attrs.str.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding str attribute " << it->first << ", " << it->second << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<string> value_input = { it->second };
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding str attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			}
			else {
				cerr << "Adding str attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//################
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else {
			cerr << "Adding str attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

//array attributes
	for( auto it = attrs.str_arr.begin(); it != attrs.str_arr.end(); it++ ) {
		status = H5Aexists_by_name( fileid, loc_name.c_str(), it->first.c_str(), 0 );
		if ( status > 0 ) {
			cerr << "Adding str attribute " << it->first << " to loc_name " << loc_name << " rejected because an attribute with such a name exists !" << "\n";
		}
		else if ( status == 0 ) {
			vector<string> value_input = it->second;
			if ( nexus_write_attribute_value( it->first, value_input ) == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
				cout << "Adding str attribute " << it->first << " to loc_name " << loc_name << " success" << "\n";
#endif
			}
			else {
				cerr << "Adding str attribute " << it->first << " to loc_name " << loc_name << " failed!" << "\n";
				//##MK::##############
				return MYHDF5_ATTRWRITE_FAILED;
			}
		}
		else {
			cerr << "Adding str attribute " << it->first << " to loc_name " << loc_name << " failed, upon existence check !" << "\n";
			return MYHDF5_ATTRACCESS_FAILED;
		}
	}

	return MYHDF5_SUCCESS;
}



vector<string> HdfFiveSeqHdl::split_absolute_path( const string h5_absolute_path )
{
	vector<string> retval = vector<string>();
	char delimiter = '/';
	string fwslash = "/";

#ifdef MYHDF5_VERBOSE_LEVEL_ONE
	cout << "__" << h5_absolute_path << "__" << "\n";
#endif

#ifdef MYHDF5_VERBOSE_LEVEL_TWO
	cout << "h5_absolute_path.length() " << h5_absolute_path.length() << "\n";
#endif
	if ( h5_absolute_path.length() > 0 ) {
		size_t idx_s = 0;
		size_t idx_e = h5_absolute_path.length();
		//remove eventually preceeding and/or trailing fwslash
		if (h5_absolute_path.rfind(delimiter, 0) != string::npos) {
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "/ at start" << "\n";
#endif
			idx_s++;
		}
		if (h5_absolute_path.compare(h5_absolute_path.length()-1, 1, fwslash) == 0) {
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		  cout << "/ at end" << "\n";
#endif
		  idx_e--;
		}
		stringstream sstream("");
		for ( size_t i = idx_s; i < idx_e; ++i) {
			sstream << h5_absolute_path.at(i);
		}
		string tmp = "";
		while( getline(sstream, tmp, delimiter) ) {
			retval.push_back(tmp);
		}
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "retval.size() " << retval.size() << "\n";
#endif
		/*
		cout << "debugging parts" << "\n";
		for( size_t i = 0; i < parts.size(); i++ ) {
			cout << parts[i] << "\n";
		}*/
	}
	return retval;
}


int HdfFiveSeqHdl::nexus_write_group( const string grpnm, ioAttributes const & attrs )
{
	if ( nexus_open( H5F_ACC_RDWR ) == H5I_INVALID_HID ) {
		cerr << "Opening file during write group attempt failed!" << "\n";
		return MYHDF5_FOPEN_FAILED;
	}
	//now the file is open otherwise we wouldnt have gotten here

	//check if the path in the open file pointed to by fileid exists
	vector<string> level_by_level = split_absolute_path( grpnm );

	//start at the file, root
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
	cout << "Checking a chain of hopefully existing links..." << "\n";
#endif
	hid_t curr_loc_id = fileid;
	string curr_group = "";

	//##MK::https://gist.github.com/jzrake/3025642
	for( size_t i = 0; i < level_by_level.size(); i++ ) {
		curr_group = level_by_level[i]; //nodes_visited += "/" + graph_nodes[i];
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "Currently visiting " << curr_group << "\n";
#endif
		htri_t status = H5Lexists( curr_loc_id, curr_group.c_str(), H5P_DEFAULT );
		if( status > 0 ) { //exists
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << curr_group << " --> exists" << "\n";
#endif
			hid_t next_loc_id = H5Gopen( curr_loc_id, curr_group.c_str(), H5P_DEFAULT );
			if ( next_loc_id == H5I_INVALID_HID ) {
				if ( curr_loc_id != fileid ) {
					if ( H5Gclose( curr_loc_id ) >= 0 ) {
						//nop
					}
					else {
						if ( H5Fclose( fileid ) >= 0 ) {
							cerr << "Successful H5Fclose after a failed H5Gclose while preparing walk after " << curr_group << "\n";
							return MYHDF5_GCLOSE_FAILED;
						}
						cerr << "Failed H5Fclose after a failed H5Gclose while preparing walk after " << curr_group << "\n";
						return MYHDF5_FCLOSE_FAILED;
					}
				}
				if ( H5Fclose( fileid ) >= 0 ) {
					cerr << "Successful H5Fclose after failed H5Gopen on " << curr_group << "\n";
					return MYHDF5_GOPEN_FAILED;
				}
				cerr << "Failed H5Fclose after failed H5Gopen on " << curr_group << "\n";
				return MYHDF5_FCLOSE_FAILED;
			}
			else {
				if ( curr_loc_id != fileid ) {

					if ( i == (level_by_level.size() - 1) ) {
						//open
						dsetid = next_loc_id; //this line is a hack that should be implemented cleaner
						//nexus_write_attributes do not open resources but assume that there is an opened
						//loc_id with a valid location id handler dsetid against which attributes are written
						//usually the locations are datasets but here we wish these to be groups and thus
						//we need to pass the class this information, the attribute write function do
						//not currently and must not close the resources pointed to my dsetid!only the
						//they only close the locally created handles for the attributes
						int attr_status = nexus_write_attributes( grpnm, attrs );
						if ( attr_status == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
							cout << "Adding attributes to " << grpnm << " was successful" << "\n";
#endif
						}
						else {
							cerr << "Adding attributes to " << grpnm << " failed!" << "\n";
						}
					}

					if ( H5Gclose( curr_loc_id ) >= 0 ) {
						//nop
					}
					else {
						if ( H5Fclose( fileid ) >= 0 ) {
							cerr << "Successful H5Fclose after a failed H5Gclose while preparing walk after >0 " << curr_group << "\n";
							return MYHDF5_GCLOSE_FAILED;
						}
						cerr << "Failed H5Fclose after a failed H5Gclose while preparing walk after >0 " << curr_group << "\n";
						return MYHDF5_FCLOSE_FAILED;
					}
				}
				curr_loc_id = next_loc_id;
				continue;
			}
		}
		else if ( status == 0 ) {
			//no error, library states that group under nodes_visited does not exist, so create it
			hid_t next_loc_id = H5Gcreate2(curr_loc_id, curr_group.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
			if ( next_loc_id == H5I_INVALID_HID ) {
				if ( curr_loc_id != fileid ) {
					if ( H5Gclose( curr_loc_id ) >= 0 ) {
						//nop
					}
					else {
						if ( H5Fclose( fileid ) >= 0 ) {
							cerr << "Successful H5Fclose after a failed H5Gclose while preparing walk after ==0 " << curr_group << "\n";
							return MYHDF5_GCLOSE_FAILED;
						}
						cerr << "Failed H5Fclose after a failed H5Gclose while preparing walk after ==0 " << curr_group << "\n";
						return MYHDF5_FCLOSE_FAILED;
					}
				}
				if ( H5Fclose( fileid ) >= 0 ) {
					cerr << "Successful H5Fclose after failed creating group ==0 " << curr_group << "\n";
					return MYHDF5_GCREATE_FAILED;
				}
				cerr << "Failed H5Fclose after failed create group ==0 " << curr_group << "\n";
				return MYHDF5_FCLOSE_FAILED;
			}
			else {
				if ( curr_loc_id != fileid || level_by_level.size() <= 1 ) {
					if ( i == (level_by_level.size() - 1) ) {
						dsetid = next_loc_id;
						int attr_status = nexus_write_attributes( grpnm, attrs );
						if ( attr_status == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
							cout << "Adding attributes to " << grpnm << " was successful" << "\n";
#endif
						}
						else {
							cerr << "Adding attributes to " << grpnm << " failed!" << "\n";
						}
					}

					if ( curr_loc_id != fileid ) {
						if ( H5Gclose( curr_loc_id ) >= 0 ) {
							//nop
						}
						else {
							if ( H5Fclose( fileid ) >= 0 ) {
								cerr << "Successful H5Fclose after a failed H5Gclose after creating sub-group in " << curr_group << "\n";
								return MYHDF5_GCLOSE_FAILED;
							}
							cerr << "Failed H5Fclose after a failed H5Gclose after creating sub-group in " << curr_group << "\n";
							return MYHDF5_FCLOSE_FAILED;
						}
					}
					else {
						//nothing to do next line will hand over the handle which will then be closed before exiting this function
					}
				}
				curr_loc_id = next_loc_id;
				continue;
			}
		}
		else { //status < 0
			cerr << "htri_t library error" << "\n";
			if ( curr_loc_id != fileid ) {
				if ( H5Gclose( curr_loc_id ) >= 0 ) {
					//nop
				}
				else {
					if ( H5Fclose( fileid ) >= 0 ) {
						cerr << "Successful H5Fclose after a failed H5Gclose when link did not exist " << curr_group << "\n";
						return MYHDF5_GCLOSE_FAILED;
					}
					cerr << "Failed H5Fclose after a failed H5Gclose when link did not exist " << curr_group << "\n";
					return MYHDF5_FCLOSE_FAILED;
				}
			}
			break;
		}
	}

	//##MK::add the annotations in the last group

	//success is achieved only when you have not previous run into a failure
	//such as having to stop the walk because of a missing path
	if ( H5Gclose( curr_loc_id ) < 0 ) {
		cerr << "Failed H5Gclose during cleanup" << "\n";
		return MYHDF5_GCLOSE_FAILED;
	}
	if ( H5Fclose( fileid ) < 0 ) {
		cerr << "Failed H5Fclose during cleanup" << "\n";
		return MYHDF5_FCLOSE_FAILED;
	}

	return MYHDF5_SUCCESS;
}


template<typename T> int HdfFiveSeqHdl::nexus_write( const string dsnm, T val, ioAttributes const & attrs )
{
	vector<T> tmp = vector<T>( 1, val );
	return nexus_write( dsnm, io_info(), tmp, attrs );
}

template int HdfFiveSeqHdl::nexus_write( const string dsnm, unsigned char val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, char val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, unsigned short val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, short val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, unsigned int val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, int val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, unsigned long val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, long val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, float val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, double val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, string val, ioAttributes const & attrs );


/*
template<typename T> int HdfFiveSeqHdl::nexus_write( const string dsnm, T & val, ioAttributes const & attrs )
{
	vector<T> tmp = vector<T>( 1, val );
	return nexus_write( dsnm, io_info(), tmp, attrs );
}

template int HdfFiveSeqHdl::nexus_write( const string dsnm, unsigned char & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, char & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, unsigned short & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, short & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, unsigned int & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, int & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, unsigned long & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, long & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, float & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, double & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, string & val, ioAttributes const & attrs );
*/


template<typename T> int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<T> & val, ioAttributes const & attrs )
{
	//open file in read/write mode
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
	cout << "nexus_write array version val.size() " << val.size() << "\n";
#endif
	if ( val.size() > 0 ) { //##MK::this should not be necessary
		size_t n_values = 1;
		for( auto it = ifo.shape.begin(); it != ifo.shape.end(); it++ ) {
			n_values *= *it;
		}
		if constexpr(std::is_same<T, string>::value) {
			if ( ifo.shape.size() == 0 && val.size() == 1 ) {
				//nop
			}
			else {
				cerr << "Writing " << dsnm << " failed, currently only single string arguments supported !" << "\n";
				return MYHDF5_NOT_IMPLEMENTED;
			}
		}
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "n_values " << n_values << "\n";
#endif
		size_t max_characters = 0;
		if ( ifo.is_valid == true && val.size() != 0 && val.size() >= n_values ) {
			hid_t dtype = 0; //H5T_STD_U32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "dtype " << dtype << "\n";
#endif
			if constexpr(std::is_same<T, unsigned char>::value) {
				dtype = H5T_STD_U8LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "dtype u8 " << dtype << "\n";
#endif
			}
			else if constexpr(std::is_same<T, char>::value) {
				dtype = H5T_STD_I8LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "dtype i8 " << dtype << "\n";
#endif
			}
			else if constexpr(std::is_same<T, unsigned short>::value) {
				dtype = H5T_STD_U16LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "dtype u16 " << dtype << "\n";
#endif
			}
			else if constexpr(std::is_same<T, short>::value) {
				dtype = H5T_STD_I16LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "dtype i16 " << dtype << "\n";
#endif
			}
			else if constexpr(std::is_same<T, unsigned int>::value) {
				dtype = H5T_STD_U32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "dtype u32 " << dtype << "\n";
#endif
			}
			else if constexpr(std::is_same<T, int>::value) {
				dtype = H5T_STD_I32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "dtype i32 " << dtype << "\n";
#endif
			}
			else if constexpr(std::is_same<T, unsigned long>::value) {
				dtype = H5T_STD_U64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "dtype u64 " << dtype << "\n";
#endif
			}
			else if constexpr(std::is_same<T, long>::value) {
				dtype = H5T_STD_I64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "dtype i64 " << dtype << "\n";
#endif
			}
			else if constexpr(std::is_same<T, float>::value) {
				dtype = H5T_IEEE_F32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "dtype f32 " << dtype << "\n";
#endif
			}
			else if constexpr(std::is_same<T, double>::value) {
				dtype = H5T_IEEE_F64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "dtype f64 " << dtype << "\n";
#endif
			}
			else if constexpr(std::is_same<T, string>::value) {
				/*
				for( auto it = val.begin(); it != val.end(); it++ ) {
					if ( it->length() < max_characters )
						continue;
					else
						max_characters = it->length();
				}
				*/
				max_characters = val[0].length(); // + 1; //include null terminator
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
				cout << "dims " << val.size() << "\n";
				cout << "sdims " << max_characters << "\n";
#endif
				if ( max_characters >= 1 ) {
					dtype = H5Tcopy(H5T_C_S1);
					if ( dtype != H5I_INVALID_HID ) {
						if ( H5Tset_size(dtype, max_characters) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
							cout << "dtype str " << dtype << "\n";
#endif
						}
						else {
							cerr << "Writing " << dsnm << " failed, dtype set size !" << "\n";
							return MYHDF5_NOT_EXPECTED;
						}
					}
					else {
						cerr << "Writing " << dsnm << " failed, dtype invalid !" << "\n";
						return MYHDF5_NOT_EXPECTED;
					}
				}
				else {
					cerr << "Writing " << dsnm << " failed, dtype max_characters !" << "\n";
					return MYHDF5_NOT_EXPECTED;
				}
			}
			else {
				//nop
			}
			if ( nexus_open( H5F_ACC_RDWR ) != H5I_INVALID_HID ) {
				//if ( nexus_dataset_exists( dsnm, true ) == false ) {
					if ( ifo.chunk.size() == 0 ) { //no chunking but contiguous layout
						if (ifo.shape.size() == 0 ) { //scalar
							int rank = 1;
							hsize_t dims[1] = { 1 };
							hsize_t maxdims[1] = { 1 };
							if constexpr(std::is_same<T, string>::value) {
								dims[0] = { 1 }; //max_characters };
								maxdims[0] = { 1 }; //max_characters };
							}
							dspcid = H5Screate_simple( rank, dims, maxdims ); // maxdims = NULL
							if ( dspcid != H5I_INVALID_HID ) {
								dsetid = H5Dcreate2( fileid, dsnm.c_str(), dtype, dspcid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
								if ( dsetid != H5I_INVALID_HID ) {
									if constexpr(std::is_same<T, string>::value) {
										//char wbuf[] = { val.at(0).c_str() };
										if ( H5Dwrite( dsetid, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, val.at(0).c_str() ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
											cout << "Writing " << dsnm << " contiguous, scalar string success" << "\n";
#endif

											int attr_status = nexus_write_attributes( dsnm, attrs );
											if ( attr_status == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
												cout << "Writing " << dsnm << " contiguous, scalar string attributes success !" << "\n";
#endif
											}
											else {
												cerr << "Writing " << dsnm << " contiguous, scalar string attributes failed " << attr_status << "\n";
											}
											return nexus_close( MYHDF5_SUCCESS );
										}
									}
									else {
										if ( H5Dwrite( dsetid, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, val.data() ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
											cout << "Writing " << dsnm << " contiguous, scalar non-string success" << "\n";
#endif

											int attr_status = nexus_write_attributes( dsnm, attrs );
											if ( attr_status == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
												cout << "Writing " << dsnm << " contiguous, scalar non-string attributes success !" << "\n";
#endif
											}
											else {
												cerr << "Writing " << dsnm << " contiguous, scalar non-string attributes failed " << attr_status << "\n";
											}
											return nexus_close( MYHDF5_SUCCESS );
										}
									}
									cerr << "Writing " << dsnm << " contiguous, scalar failed !" << "\n";
									return nexus_close( MYHDF5_DSETWRITE_FAILED );
								}
								cerr << "Creating " << dsnm << " contiguous, scalar dset failed !" << "\n";
								return nexus_close( MYHDF5_DSETOPEN_FAILED );
							}
							cerr << "Creating " << dsnm << " contiguous, scalar dspc failed !" << "\n";
							return nexus_close( MYHDF5_DSPCOPEN_FAILED );
						}
						else if ( ifo.shape.size() == 1 ) { //1d, vector
							int rank = 1;
							hsize_t dims[1] = { ifo.shape[0] };
							hsize_t maxdims[1] = { ifo.shape[0] };
							dspcid = H5Screate_simple( rank, dims, maxdims );
							if ( dspcid != H5I_INVALID_HID ) {
								dsetid = H5Dcreate2( fileid, dsnm.c_str(), dtype, dspcid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
								if ( dsetid != H5I_INVALID_HID ) {
									if ( H5Dwrite( dsetid, dtype, dspcid, dspcid, H5P_DEFAULT, val.data() ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
										cout << "Writing " << dsnm << " contiguous, 1d success" << "\n";
#endif

										int attr_status = nexus_write_attributes( dsnm, attrs );
										if ( attr_status == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
											cout << "Writing " << dsnm << " contiguous, 1d attributes success !" << "\n";
#endif
										}
										else {
											cerr << "Writing " << dsnm << " contiguous, 1d attributes failed " << attr_status << "\n";
										}
										return nexus_close( MYHDF5_SUCCESS );
									}
									cerr << "Writing " << dsnm << " contiguous, 1d failed !" << "\n";
									return nexus_close( MYHDF5_DSETWRITE_FAILED );
								}
								cerr << "Creating " << dsnm << " contiguous, 1d dset failed !" << "\n";
								return nexus_close( MYHDF5_DSETOPEN_FAILED );
							}
							cerr << "Creating " << dsnm << " contiguous, 1d dspc failed !" << "\n";
							return nexus_close( MYHDF5_DSPCOPEN_FAILED );
						}
						else if ( ifo.shape.size() == 2 ) { //2d, matrix
							int rank = 2;
							hsize_t dims[2] = { ifo.shape[0], ifo.shape[1] };
							hsize_t maxdims[2] = { ifo.shape[0], ifo.shape[1] };

							hsize_t offset[2] = { 0, 0 }; //hyperslab design
							hsize_t count[2] = { ifo.shape[0], ifo.shape[1] };
							hsize_t stride[2] = { 1, 1 };
							hsize_t block[2] = { 1, 1 };

							dspcid = H5Screate_simple( rank, dims, maxdims );
							if ( dspcid != H5I_INVALID_HID ) {
								dsetid = H5Dcreate2( fileid, dsnm.c_str(), dtype, dspcid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
								if ( dsetid != H5I_INVALID_HID ) {
									if ( H5Sselect_hyperslab(dspcid, H5S_SELECT_SET, offset, stride, count, block) >= 0 ) {
										if ( H5Dwrite( dsetid, dtype, dspcid, dspcid, H5P_DEFAULT, val.data() ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
											cout << "Writing " << dsnm << " contiguous, 2d success" << "\n";
#endif

											int attr_status = nexus_write_attributes( dsnm, attrs );
											if ( attr_status == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
												cout << "Writing " << dsnm << " contiguous, 2d attributes success !" << "\n";
#endif
											}
											else {
												cerr << "Writing " << dsnm << " contiguous, 2d attributes failed " << attr_status << "\n";
											}
											return nexus_close( MYHDF5_SUCCESS );
										}
										cerr << "Writing " << dsnm << " contiguous, 2d failed !" << "\n";
										return nexus_close( MYHDF5_DSETWRITE_FAILED );
									}
									cerr << "Creating " << dsnm << " contiguous, hslab 2d failed !" << "\n";
									return nexus_close( MYHDF5_DSETWRITE_FAILED );
								}
								cerr << "Creating " << dsnm << " contiguous, 2d dset failed !" << "\n";
								return nexus_close( MYHDF5_DSETOPEN_FAILED );
							}
							cerr << "Creating " << dsnm << " contiguous, 2d dspc failed !" << "\n";
							return nexus_close( MYHDF5_DSPCOPEN_FAILED );
						}
						cerr << "Creating " << dsnm << " failed, because d > 2 arrays not supported !" << "\n";
						return nexus_close( MYHDF5_NOT_IMPLEMENTED );
					} //done with contiguous
					else { // chunked layout, eventually using compression
						if ( ifo.shape.size() == 0 ) { //scalar
							cerr << "Writing " << dsnm << " failed, as scalar values are never stored chunked !" << "\n";
							return nexus_close( MYHDF5_NOT_IMPLEMENTED );
						}
						else if ( ifo.shape.size() == 1 ) { //1d, vector
							//uncomment these variables to use SZIP compression
							//unsigned szip_options_mask;
							//unsigned szip_pixels_per_block;
							int rank = 1;
							hsize_t dims[1] = { ifo.shape[0] };
							hsize_t maxdims[1] = { ifo.shape[0] };
							dspcid = H5Screate_simple( rank, dims, maxdims ); //maxdims = NULL
							if ( dspcid != H5I_INVALID_HID ) {
								plistid = H5Pcreate( H5P_DATASET_CREATE );
								if ( plistid != H5I_INVALID_HID ) {
									int chunk_rank = 1;
									hsize_t chunk_dims[1] = { ifo.chunk[0] };
									if ( H5Pset_chunk( plistid, chunk_rank, chunk_dims ) >= 0 ) {
										if ( ifo.compression == MYHDF5_COMPRESSION_GZIP ) {
											if ( H5Pset_deflate( plistid, (unsigned) ifo.compression_opts ) >= 0 ) {
												//set ZLIB / DEFLATE Compression using compression level 6.
												//szip_options_mask = H5_SZIP_NN_OPTION_MASK;
												//szip_pixels_per_block = 16;
												//status = H5Pset_szip (plistid, szip_options_mask, szip_pixels_per_block);
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
												cout << "Setting deflate " << dsnm << " chunked, 1d" << "\n";
#endif
											}
											else {
												cerr << "Creating " << dsnm << " chunked, 1d set deflate failed !" << "\n";
												return nexus_close( MYHDF5_PLSTACCESS_FAILED );
											}
										}
										dsetid = H5Dcreate2( fileid, dsnm.c_str(), dtype, dspcid, H5P_DEFAULT, plistid, H5P_DEFAULT );
										if ( dsetid != H5I_INVALID_HID ) {
											if ( H5Dwrite( dsetid, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, val.data() ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
												cout << "Writing " << dsnm << " chunked, 1d success" << "\n";
#endif

												int attr_status = nexus_write_attributes( dsnm, attrs );
												if ( attr_status == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
													cout << "Writing " << dsnm << " contiguous, 2d attributes success !" << "\n";
#endif
												}
												else {
													cerr << "Writing " << dsnm << " contiguous, 2d attributes failed " << attr_status << "\n";
												}
												return nexus_close( MYHDF5_SUCCESS );
											}
											cerr << "Writing " << dsnm << " chunked, 1d create failed !" << "\n";
											return nexus_close( MYHDF5_DSETWRITE_FAILED );
										}
										cerr << "Writing " << dsnm << " chunked, 1d create failed !" << "\n";
										return nexus_close( MYHDF5_DSETWRITE_FAILED );
									}
									cerr << "Creating " << dsnm << " chunked, 1d set chunk failed !" << "\n";
									return nexus_close( MYHDF5_DSETWRITE_FAILED );
								}
								cerr << "Creating " << dsnm << " chunked, 1d filter create failed !" << "\n";
								return nexus_close( MYHDF5_PLSTOPEN_FAILED );
							}
							cerr << "Creating " << dsnm << " chunked, 1d set dspc failed !" << "\n";
							return nexus_close( MYHDF5_DSPCOPEN_FAILED );
						}
						else if ( ifo.shape.size() == 2 ) { //2d, matrix
							//uncomment these variables to use SZIP compression
							//unsigned szip_options_mask;
							//unsigned szip_pixels_per_block;
							int rank = 2;
							hsize_t dims[2] = { ifo.shape[0], ifo.shape[1] };
							hsize_t maxdims[2] = { ifo.shape[0], ifo.shape[1] };
							dspcid = H5Screate_simple( rank, dims, maxdims ); //maxdims = NULL
							if ( dspcid != H5I_INVALID_HID ) {
								plistid = H5Pcreate( H5P_DATASET_CREATE );
								if ( plistid != H5I_INVALID_HID ) {
									int chunk_rank = 2;
									hsize_t chunk_dims[2] = { ifo.chunk[0], ifo.chunk[1] };
									if ( H5Pset_chunk( plistid, chunk_rank, chunk_dims ) >= 0 ) {
										if ( ifo.compression == MYHDF5_COMPRESSION_GZIP ) {
											if ( H5Pset_deflate( plistid, (unsigned) ifo.compression_opts ) >= 0 ) {
												//set ZLIB / DEFLATE Compression using compression level 6.
												//szip_options_mask = H5_SZIP_NN_OPTION_MASK;
												//szip_pixels_per_block = 16;
												//status = H5Pset_szip (plistid, szip_options_mask, szip_pixels_per_block);
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
												cout << "Setting deflate " << dsnm << " chunked, 2d" << "\n";
#endif
											}
											else {
												cerr << "Creating " << dsnm << " chunked, 2d set deflate failed !" << "\n";
												return nexus_close( MYHDF5_PLSTACCESS_FAILED );
											}
										}
										dsetid = H5Dcreate2( fileid, dsnm.c_str(), dtype, dspcid, H5P_DEFAULT, plistid, H5P_DEFAULT );
										if ( dsetid != H5I_INVALID_HID ) {
											if ( H5Dwrite( dsetid, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, val.data() ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
												cout << "Writing " << dsnm << " chunked, 2d success" << "\n";
#endif

												//##MK::write attributes
												int attr_status = nexus_write_attributes( dsnm, attrs );
												if ( attr_status == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
													cout << "Writing " << dsnm << " contiguous, 2d attributes success !" << "\n";
#endif
												}
												else {
													cerr << "Writing " << dsnm << " contiguous, 2d attributes failed " << attr_status << "\n";
												}
												return nexus_close( MYHDF5_SUCCESS );
											}
											cerr << "Writing " << dsnm << " chunked, 2d failed !" << "\n";
											return nexus_close( MYHDF5_DSETWRITE_FAILED );
										}
										cerr << "Writing " << dsnm << " chunked, 2d failed  !" << "\n";
										return nexus_close( MYHDF5_DSETWRITE_FAILED );
									}
									cerr << "Creating " << dsnm << " chunked, 2d chunking failed !" << "\n";
									return nexus_close( MYHDF5_DSETWRITE_FAILED );
								}
								cerr << "Creating " << dsnm << " chunked, 2d plist failed !" << "\n";
								return nexus_close( MYHDF5_PLSTOPEN_FAILED );
							}
							cerr << "Creating " << dsnm << " chunked, 2d dspc failed !" << "\n";
							return nexus_close( MYHDF5_DSPCOPEN_FAILED );
						}
						//ifo.shape.size() > 2 not supported !
						cerr << "Creating " << dsnm << " failed because ifo.shape.size() > 2 not implemented !" << "\n";
						return nexus_close( MYHDF5_NOT_IMPLEMENTED );
					} //done with chunked
				//} //done newly created dataset
				//cerr << "Creating " << dsnm << " failed because dsnm already exists !" << "\n";
				//return nexus_close( MYHDF5_ERROR_OBJ_EXISTS );
			}
			cerr << "Opening file " << h5resultsfn << " failed !" << "\n";
			return nexus_close( MYHDF5_FOPEN_FAILED );
		}
		cerr << "H5Core error on input size " << h5resultsfn << " failed !" << "\n";
		return MYHDF5_ERROR_INPUT_SIZE;
	}

	cerr << "WARNING: value array val for dataset " << dsnm << " is empty!" << "\n";
	return MYHDF5_SUCCESS;
}

template int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<unsigned char> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<char> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<unsigned short> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<short> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<unsigned int> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<int> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<unsigned long> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<long> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<float> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<double> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write( const string dsnm, io_info const & ifo, vector<string> & val, ioAttributes const & attrs );


template<typename T> int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, io_info const & ifo, vector<T> & val, ioAttributes const & attrs )
{
	//open file in read/write mode
	string dset_case = "";
	switch(ifo.shape.size())
	{
		case 0:
			dset_case = "()"; break;
		case 1:
			dset_case = "(i,)"; break;
		case 2:
			dset_case = "(i, j)"; break;
		case 3:
			dset_case = "(i, j, k)"; break;
		case 4:
			dset_case = "(i, j, k, l)"; break;
		default:
			cerr << "Hitting an unimplemented case!" << "\n";
			return MYHDF5_FAILED;
	}
	if ( ifo.chunk.size() == 0 ) {
		dset_case = dset_case + ", contiguous layout";
	}
	else if ( ifo.chunk.size() == ifo.shape.size() ) {
		dset_case = dset_case + ", chunked layout";
	}
	else {
		cerr << "Hitting an unsupported chunking scenario!" << "\n";
		return MYHDF5_FAILED;
	}

	size_t n_values = 1;
	for( auto it = ifo.shape.begin(); it != ifo.shape.end(); it++ ) {
		n_values *= *it;
	}

	if ( n_values != val.size() ) {
		cerr << "Dataset shape and size of val.data() do not match!" << "\n";
		return MYHDF5_FAILED;
	}

	if ( n_values == 0 ) {
		cerr << "Dataset is empty!" << "\n";
		return MYHDF5_FAILED;
	}

#ifdef MYHDF5_VERBOSE_LEVEL_TWO
	cout << "nexus_write_at_once array version val.size() " << val.size() << "\n";
	cout << "dset_case " << dset_case << "\n";
#endif

	hid_t dtype = 0;
	if constexpr(std::is_same<T, unsigned char>::value) {
		dtype = H5T_STD_U8LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype u8 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, char>::value) {
		dtype = H5T_STD_I8LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype i8 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, unsigned short>::value) {
		dtype = H5T_STD_U16LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype u16 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, short>::value) {
		dtype = H5T_STD_I16LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype i16 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, unsigned int>::value) {
		dtype = H5T_STD_U32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype u32 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, int>::value) {
		dtype = H5T_STD_I32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype i32 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, unsigned long>::value) {
		dtype = H5T_STD_U64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype u64 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, long>::value) {
		dtype = H5T_STD_I64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype i64 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, float>::value) {
		dtype = H5T_IEEE_F32LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype f32 " << dtype << "\n";
#endif
	}
	else if constexpr(std::is_same<T, double>::value) {
		dtype = H5T_IEEE_F64LE;
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
		cout << "dtype f64 " << dtype << "\n";
#endif
	}
	else {
		cerr << "Hitting an unsupported datatype!" << "\n";
		return MYHDF5_FAILED;
	}

	if ( nexus_open( H5F_ACC_RDWR ) != H5I_INVALID_HID ) {  //if ( nexus_dataset_exists( dsnm, true ) == false ) {
		int rank = 0;
		int chkrank = 0;
		vector<hsize_t> dims;
		vector<hsize_t> maxdims;
		vector<hsize_t> chkdims;
		if ( (ifo.chunk.size() == 0) || (ifo.chunk.size() != 0 && ifo.shape.size() == 0) ) {
			//contiguous layout used and catching the case that scalars are never using chunking
			if (ifo.shape.size() == 0 ) { //scalar
				rank = 1;
				dims.push_back(1);
				maxdims.push_back(1);
			}
			else {
				for( auto it = ifo.shape.begin(); it != ifo.shape.end(); it++ ) {
					rank++;
					dims.push_back(*it);
					maxdims.push_back(*it);
				}
			}
			dspcid = H5Screate_simple( rank, dims.data(), maxdims.data() ); // maxdims = NULL
			if ( dspcid != H5I_INVALID_HID ) {
				dsetid = H5Dcreate2( fileid, dsnm.c_str(), dtype, dspcid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
				if ( dsetid != H5I_INVALID_HID ) {
					int write_status = -1;
					if ( rank == 1 ) {
						write_status = H5Dwrite( dsetid, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, val.data() );
					}
					else {
						write_status = H5Dwrite( dsetid, dtype, dspcid, dspcid, H5P_DEFAULT, val.data() );
					}

					if ( write_status >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
						cout << "Writing " << dsnm << " " << dset_case << " success, dataset" << "\n";
#endif
						int attr_status = nexus_write_attributes( dsnm, attrs );
						if ( attr_status == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
							cout << "Writing " << dsnm << " " << dset_case << " success, attributes" << "\n";
#endif
						}
						else {
							cerr << "Writing " << dsnm << " " << dset_case << " failed, attributes !" << "\n";
						}
						return nexus_close( MYHDF5_SUCCESS );
					}
					cerr << "Writing " << dsnm << " " << dset_case << " failed !" << "\n";
					return nexus_close( MYHDF5_DSETWRITE_FAILED );
				}
				cerr << "Creating dset " << dsnm << " " << dset_case << " failed !" << "\n";
				return nexus_close( MYHDF5_DSETOPEN_FAILED );
			}
			cerr << "Creating dspc " << dsnm << " " << dset_case << " failed !" << "\n";
			return nexus_close( MYHDF5_DSPCOPEN_FAILED );
		} //done with contiguous
		else {
			//chunked layout
			for( auto it = ifo.shape.begin(); it != ifo.shape.end(); it++ ) {
				rank++;
				dims.push_back(*it);
				maxdims.push_back(*it);
			}
			for( auto jt = ifo.chunk.begin(); jt != ifo.chunk.end(); jt++ ) {
				chkrank++;
				chkdims.push_back(*jt);
			}
			dspcid = H5Screate_simple( rank, dims.data(), maxdims.data() );
			if ( dspcid != H5I_INVALID_HID ) {
				plistid = H5Pcreate( H5P_DATASET_CREATE );
				if ( plistid != H5I_INVALID_HID ) {
					if ( H5Pset_chunk( plistid, chkrank, chkdims.data() ) >= 0 ) {
						if ( ifo.compression == MYHDF5_COMPRESSION_GZIP ) {
							if ( H5Pset_deflate( plistid, (unsigned) ifo.compression_opts ) >= 0 ) {
								//set ZLIB / DEFLATE Compression using compression level 6.
								//szip_options_mask = H5_SZIP_NN_OPTION_MASK;
								//szip_pixels_per_block = 16;
								//status = H5Pset_szip (plistid, szip_options_mask, szip_pixels_per_block);
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
								cout << "Setting deflate " << dsnm << " " << dset_case << " success" << "\n";
#endif
							}
							else {
								cerr << "Setting deflate " << dsnm << " " << dset_case << " failed !" << "\n";
								return nexus_close( MYHDF5_PLSTACCESS_FAILED );
							}
						}
						dsetid = H5Dcreate2( fileid, dsnm.c_str(), dtype, dspcid, H5P_DEFAULT, plistid, H5P_DEFAULT );
						if ( dsetid != H5I_INVALID_HID ) {
							if ( H5Dwrite( dsetid, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, val.data() ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
								cout << "Writing " << dsnm << " " << dset_case << " success" << "\n";
#endif
								int attr_status = nexus_write_attributes( dsnm, attrs );
								if ( attr_status == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
									cout << "Writing attributes " << dsnm << " " << dset_case << " success, attributes" << "\n";
#endif
								}
								else {
									cerr << "Writing attributes " << dsnm << " " << dset_case << " failed, attributes !" << "\n";
									//complain!
								}
								return nexus_close( MYHDF5_SUCCESS );
							}
							cerr << "Writing " << dsnm << " " << dset_case << " failed !" << "\n";
							return nexus_close( MYHDF5_DSETWRITE_FAILED );
						}
						cerr << "Creating dset " << dsnm << " " << dset_case << " failed !" << "\n";
						return nexus_close( MYHDF5_DSETWRITE_FAILED );
					}
					cerr << "Setting chunk size " << dsnm << " " << dset_case << " failed !" << "\n";
					return nexus_close( MYHDF5_PLSTOPEN_FAILED ); //##MK::different error code
				}
				cerr << "Opening plist " << dsnm << " " << dset_case << " failed !" << "\n";
				return nexus_close( MYHDF5_PLSTOPEN_FAILED );
			}
			cerr << "Creating dspc " << dsnm << " " << dset_case << " failed !" << "\n";
			return nexus_close( MYHDF5_DSPCOPEN_FAILED );
		} //done with chunked layout
	}
	cerr << "Opening file " << h5resultsfn << " failed !" << "\n";
	return nexus_close( MYHDF5_FOPEN_FAILED );
}


template int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, io_info const & ifo, vector<unsigned char> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, io_info const & ifo, vector<char> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, io_info const & ifo, vector<unsigned short> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, io_info const & ifo, vector<short> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, io_info const & ifo, vector<unsigned int> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, io_info const & ifo, vector<int> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, io_info const & ifo, vector<unsigned long> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, io_info const & ifo, vector<long> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, io_info const & ifo, vector<float> & val, ioAttributes const & attrs );
template int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, io_info const & ifo, vector<double> & val, ioAttributes const & attrs );


int HdfFiveSeqHdl::nexus_write_at_once( const string dsnm, vector<string> & val, ioAttributes const & attrs )
{
	//open file in read/write mode
	if ( val.size() != 1 ) {
		cerr << "Writing " << dsnm << " " << " supports only single strings!" << "\n";
		return MYHDF5_FAILED;
	}

	size_t max_characters = 0;
	/*
	for( auto it = val.begin(); it != val.end(); it++ ) {
		if ( it->length() < max_characters )
			continue;
		else
			max_characters = it->length();
	}
	*/
	max_characters = val[0].length(); // + 1; //include null terminator
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
	cout << "dims " << val.size() << "\n";
	cout << "sdims " << max_characters << "\n";
#endif

	if ( max_characters == 0 ) {
		cerr << "Writing " << dsnm << " string must not be empty!" << "\n";
		return MYHDF5_SUCCESS;
	}

	hid_t dtype = H5Tcopy(H5T_C_S1);
	if ( dtype != H5I_INVALID_HID ) {
		if ( H5Tset_size(dtype, max_characters) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_TWO
			cout << "dtype str " << dtype << "\n";
#endif
		}
		else {
			cerr << "Writing " << dsnm << " failed, dtype set size !" << "\n";
			return MYHDF5_NOT_EXPECTED;
		}
	}
	else {
		cerr << "Writing " << dsnm << " failed, dtype invalid !" << "\n";
		return MYHDF5_NOT_EXPECTED;
	}

	if ( nexus_open( H5F_ACC_RDWR ) != H5I_INVALID_HID ) { //if ( nexus_dataset_exists( dsnm, true ) == false ) {
		int rank = 1;
		hsize_t dims[1] = { 1 };
		hsize_t maxdims[1] = { 1 };
		dspcid = H5Screate_simple( rank, dims, maxdims );
		if ( dspcid != H5I_INVALID_HID ) {
			dsetid = H5Dcreate2( fileid, dsnm.c_str(), dtype, dspcid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
			if ( dsetid != H5I_INVALID_HID ) {
				if ( H5Dwrite( dsetid, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, val.at(0).c_str() ) >= 0 ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
					cout << "Writing " << dsnm << " string array success" << "\n";
#endif
					int attr_status = nexus_write_attributes( dsnm, attrs );
					if ( attr_status == MYHDF5_SUCCESS ) {
#ifdef MYHDF5_VERBOSE_LEVEL_ONE
						cout << "Writing " << dsnm << " string array success, attributes" << "\n";
#endif
					}
					else {
						cerr << "Writing " << dsnm << " string array failed, attributes !" << "\n";
					}
					return nexus_close( MYHDF5_SUCCESS );
				}
				cerr << "Writing " << dsnm << " string array failed !" << "\n";
				return nexus_close( MYHDF5_DSETWRITE_FAILED );
			}
			cerr << "Creating dset " << dsnm << " string array failed !" << "\n";
			return nexus_close( MYHDF5_DSETOPEN_FAILED );
		}
		cerr << "Creating dspc " << dsnm << " string array failed !" << "\n";
		return nexus_close( MYHDF5_DSPCOPEN_FAILED );
	}
	cerr << "Opening file " << h5resultsfn << " failed !" << "\n";
	return nexus_close( MYHDF5_FOPEN_FAILED );
}
