
#ifndef __RYMLYAMLINTERFACE__
#define __RYMLYAMLINTERFACE__

#include "../STLInterface.h"

#define RYML_SINGLE_HDR_DEFINE_NOW
#include "../rapidyaml/tools/ryml_all.hpp"
#include "../rapidyaml/src/c4/yml/std/string.hpp"

template<class CharContainer>
size_t file_get_contents(const char *filename, CharContainer *v)
{
    std::FILE *fp = std::fopen(filename, "rb");
    RYML_CHECK_MSG(fp != nullptr, "could not open file");
    std::fseek(fp, 0, SEEK_END);
    long sz = std::ftell(fp);
    v->resize(static_cast<typename CharContainer::size_type>(sz));
    if(sz)
    {
        std::rewind(fp);
        size_t ret = std::fread(&(*v)[0], 1, v->size(), fp);
        RYML_CHECK(ret == (size_t)sz);
    }
    std::fclose(fp);
    return v->size();
}


template<class CharContainer>
CharContainer file_get_contents(const char *filename)
{
    CharContainer cc;
    file_get_contents(filename, &cc);
    return cc;
}


//proof-of-concept
/*
template<typename T> T get_concept_i( c4::yml::ConstNodeRef & nested_dict, vector<string> const & split_path )
{
	if ( i >= 0 && i < token.size() - 1) {
		get_concept_i(nested_dict[split_path[i].c_str()], token, i + 1);
	}
	else {
		return nested_dict[token[i].c_str()];
	}
}

auto get_concept_ij( auto const & nested_dict, vector<string> const & token, const size_t i, const size_t j )
{
	if ( i >= 0 && i < token.size() - 1) {
		get_concept_ij(nested_dict[token[i].c_str()], token, i + 1, j);
	}
	else {
		return nested_dict[token[i].c_str()][j];
	}
}

void get_concept_num_children( auto & nested_dict, vector<string> const & token, const size_t i )
{
	if ( i >= 0 && i < token.size() - 1) {
		get_concept_num_children(nested_dict[token[i].c_str()], token, i + 1);
	}
	else {
		return nested_dict[token[i].c_str()].num_children();
	}
}
*/

template<typename T> T ryml_wrapper_read_value(ryml::ConstNodeRef & root, const string flat_path )
{
	T retval;
	if constexpr(std::is_same<T, string>::value) {
		retval = "";
	}
	else {
		retval = 0;
	}
	stringstream test(flat_path);
	//cout << "test " << test.str() << "\n";
	string segment;
	vector<string> levels;
	while(std::getline(test, segment, '/')) {
		levels.push_back(segment);
	}
	//cout << "levels.size() " << levels.size() << "\n";
	if ( levels.size() > 0 ) {
		auto inception = root;
		for( size_t i = 0; i < levels.size(); i++ ) {
			inception = inception[levels[i].c_str()];
		}

		if constexpr(std::is_same<T, unsigned int>::value) {
			retval = 0;
			inception >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, int>::value) {
			retval = 0;
			inception >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, float>::value) {
			retval = 0.;
			inception >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, double>::value) {
			retval = 0.;
			inception >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, string>::value) {
			retval = "";
			inception >> retval;
			return retval;
		}
	}
	return retval;
}
template unsigned int ryml_wrapper_read_value(ryml::ConstNodeRef & root, const string flat_path);
template int ryml_wrapper_read_value(ryml::ConstNodeRef & root, const string flat_path);
template float ryml_wrapper_read_value(ryml::ConstNodeRef & root, const string flat_path);
template double ryml_wrapper_read_value(ryml::ConstNodeRef & root, const string flat_path);
template string ryml_wrapper_read_value(ryml::ConstNodeRef & root, const string flat_path);

/*
template<typename T> T read_value_one_level(ryml::ConstNodeRef & root, vector<string> const & path)
{
	if ( path.size() == 1 ) {
		if constexpr(std::is_same<T, unsigned int>::value) {
			unsigned int retval = 0;
			root[path[0].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, int>::value) {
			int retval = 0;
			root[path[0].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, float>::value) {
			float retval = 0.;
			root[path[0].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, double>::value) {
			double retval = 0.;
			root[path[0].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, string>::value) {
			string retval = "";
			root[path[0].c_str()] >> retval;
			return retval;
		}
	}
	return 0;
}
template unsigned int read_value_one_level(ryml::ConstNodeRef & root, vector<string> const & path);
template int read_value_one_level(ryml::ConstNodeRef & root, vector<string> const & path);
template float read_value_one_level(ryml::ConstNodeRef & root, vector<string> const & path);
template double read_value_one_level(ryml::ConstNodeRef & root, vector<string> const & path);
template string read_value_one_level(ryml::ConstNodeRef & root, vector<string> const & path);


template<class T> T read_value_two_level(ryml::ConstNodeRef & root, vector<string> const & path)
{
	if ( path.size() == 2 ) {
		if constexpr(std::is_same<T, unsigned int>::value) {
			unsigned int retval = 0;
			root[path[0].c_str()][path[1].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, int>::value) {
			int retval = 0;
			root[path[0].c_str()][path[1].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, float>::value) {
			float retval = 0.;
			root[path[0].c_str()][path[1].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, double>::value) {
			double retval = 0.;
			root[path[0].c_str()][path[1].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, string>::value) {
			string retval = "";
			root[path[0].c_str()][path[1].c_str()] >> retval;
			return retval;
		}
	}
	return 0;
}
template unsigned int read_value_two_level(ryml::ConstNodeRef & root, vector<string> const & path);
template int read_value_two_level(ryml::ConstNodeRef & root, vector<string> const & path);
template float read_value_two_level(ryml::ConstNodeRef & root, vector<string> const & path);
template double read_value_two_level(ryml::ConstNodeRef & root, vector<string> const & path);
template string read_value_two_level(ryml::ConstNodeRef & root, vector<string> const & path);


template<class T> T read_value_three_level(ryml::ConstNodeRef & root, vector<string> const & path)
{
	if ( path.size() == 3 ) {
		if constexpr(std::is_same<T, unsigned int>::value) {
			unsigned int retval = 0;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, int>::value) {
			int retval = 0;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, float>::value) {
			float retval = 0.;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, double>::value) {
			double retval = 0.;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, string>::value) {
			string retval = "";
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()] >> retval;
			return retval;
		}
	}
	return 0;
}
template unsigned int read_value_three_level(ryml::ConstNodeRef & root, vector<string> const & path);
template int read_value_three_level(ryml::ConstNodeRef & root, vector<string> const & path);
template float read_value_three_level(ryml::ConstNodeRef & root, vector<string> const & path);
template double read_value_three_level(ryml::ConstNodeRef & root, vector<string> const & path);
template string read_value_three_level(ryml::ConstNodeRef & root, vector<string> const & path);


template<class T> T read_value_four_level(ryml::ConstNodeRef & root, vector<string> const & path)
{
	if ( path.size() == 4 ) {
		if constexpr(std::is_same<T, unsigned int>::value) {
			unsigned int retval = 0;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, int>::value) {
			int retval = 0;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, float>::value) {
			float retval = 0.;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, double>::value) {
			double retval = 0.;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, string>::value) {
			string retval = "";
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()] >> retval;
			return retval;
		}
	}
	return 0;
}
template unsigned int read_value_four_level(ryml::ConstNodeRef & root, vector<string> const & path);
template int read_value_four_level(ryml::ConstNodeRef & root, vector<string> const & path);
template float read_value_four_level(ryml::ConstNodeRef & root, vector<string> const & path);
template double read_value_four_level(ryml::ConstNodeRef & root, vector<string> const & path);
template string read_value_four_level(ryml::ConstNodeRef & root, vector<string> const & path);


template<class T> T read_value_five_level(ryml::ConstNodeRef & root, vector<string> const & path)
{
	if ( path.size() == 5 ) {
		if constexpr(std::is_same<T, unsigned int>::value) {
			unsigned int retval = 0;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][path[4].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, int>::value) {
			int retval = 0;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][path[4].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, float>::value) {
			float retval = 0.;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][path[4].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, double>::value) {
			double retval = 0.;
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][path[4].c_str()] >> retval;
			return retval;
		}
		else if constexpr(std::is_same<T, string>::value) {
			string retval = "";
			root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][path[4].c_str()] >> retval;
			return retval;
		}
	}
	return 0;
}
template unsigned int read_value_five_level(ryml::ConstNodeRef & root, vector<string> const & path);
template int read_value_five_level(ryml::ConstNodeRef & root, vector<string> const & path);
template float read_value_five_level(ryml::ConstNodeRef & root, vector<string> const & path);
template double read_value_five_level(ryml::ConstNodeRef & root, vector<string> const & path);
template string read_value_five_level(ryml::ConstNodeRef & root, vector<string> const & path);
*/

/*
template<typename T> T read_value(ryml::ConstNodeRef & root, const string flat_path)
{
	T retval;
	if constexpr(std::is_same<T, string>::value) {
		retval = "";
	}
	else {
		retval = 0;
	}
	stringstream test(flat_path);
	cout << "test " << test.str() << "\n";
	string segment;
	vector<string> levels;
	while(std::getline(test, segment, '/')) {
		levels.push_back(segment);
	}
	cout << "levels.size() " << levels.size() << "\n";
	switch(levels.size())
	{
		case 1: {
			cout << "\t" << "one_level" << "\n";
			retval = read_value_one_level<T>(root, levels);
			break;
		}
		case 2: {
			cout << "\t" << "two_level" << "\n";
			retval = read_value_two_level<T>(root, levels);
			break;
		}
		case 3: {
			cout << "\t" << "three_level" << "\n";
			retval = read_value_three_level<T>(root, levels);
			break;
		}
		case 4: {
			cout << "\t" << "four_level" << "\n";
			retval = read_value_four_level<T>(root, levels);
			break;
		}
		case 5: {
			cout << "\t" << "five_level" << "\n";
			retval = read_value_five_level<T>(root, levels);
			break;
		}
		default: {
			break;
		}
	}
	return retval;
}
template unsigned int read_value(ryml::ConstNodeRef & root, const string flat_path);
template int read_value(ryml::ConstNodeRef & root, const string flat_path);
template float read_value(ryml::ConstNodeRef & root, const string flat_path);
template double read_value(ryml::ConstNodeRef & root, const string flat_path);
template string read_value(ryml::ConstNodeRef & root, const string flat_path);
*/

template<typename T> vector<T> ryml_wrapper_read_array(ryml::ConstNodeRef const & root, const string flat_path)
{
	vector<T> retval;
	stringstream test(flat_path);  //cout << "test " << test.str() << "\n";
	string segment;
	vector<string> levels;
	while(std::getline(test, segment, '/')) {
		levels.push_back(segment);
	}
	if ( levels.size() > 0 ) {
		auto inception = root;
		for( size_t i = 0; i < levels.size(); i++ ) {
			inception = inception[levels[i].c_str()];
		}

		for ( int j = 0; j < inception.num_children(); j++ ) {
			if constexpr(std::is_same<T, unsigned int>::value) {
				unsigned int tmp = 0;
				inception[j] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, int>::value) {
				int tmp = 0;
				inception[j] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, float>::value) {
				float tmp = 0.;
				inception[j] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, double>::value) {
				double tmp = 0.;
				inception[j] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, string>::value) {
				string tmp = "";
				inception[j] >> tmp;
				retval.push_back(tmp);
			}
		}
	}
	return retval;
}
template vector<unsigned int> ryml_wrapper_read_array(ryml::ConstNodeRef const & root, const string path);
template vector<int> ryml_wrapper_read_array(ryml::ConstNodeRef const & root, const string path);
template vector<float> ryml_wrapper_read_array(ryml::ConstNodeRef const & root, const string path);
template vector<double> ryml_wrapper_read_array(ryml::ConstNodeRef const & root, const string path);
template vector<string> ryml_wrapper_read_array(ryml::ConstNodeRef const & root, const string path);

#endif


/*
template<typename T> vector<T> read_array_one_level(ryml::ConstNodeRef & root, vector<string> const & path)
{
	vector<T> retval;
	if ( path.size() == 1 ) {
		for ( int i = 0; i < root[path[0].c_str()].num_children(); i++ ) {
			if constexpr(std::is_same<T, unsigned int>::value) {
				unsigned int tmp = 0;
				root[path[0].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, int>::value) {
				int tmp = 0;
				root[path[0].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, float>::value) {
				float tmp = 0.;
				root[path[0].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, double>::value) {
				double tmp = 0.;
				root[path[0].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, string>::value) {
				string tmp = "";
				root[path[0].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
		}
	}
	return retval;
}
template vector<unsigned int> read_array_one_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<int> read_array_one_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<float> read_array_one_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<double> read_array_one_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<string> read_array_one_level(ryml::ConstNodeRef & root, vector<string> const & path);


template<class T> vector<T> read_array_two_level(ryml::ConstNodeRef & root, vector<string> const & path)
{
	vector<T> retval;
	if ( path.size() == 2 ) {
		for ( int i = 0; i < root[path[0].c_str()][path[1].c_str()].num_children(); i++ ) {
			if constexpr(std::is_same<T, unsigned int>::value) {
				unsigned int tmp = 0;
				root[path[0].c_str()][path[1].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, int>::value) {
				int tmp = 0;
				root[path[0].c_str()][path[1].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, float>::value) {
				float tmp = 0.;
				root[path[0].c_str()][path[1].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, double>::value) {
				double tmp = 0.;
				root[path[0].c_str()][path[1].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, string>::value) {
				string tmp = "";
				root[path[0].c_str()][path[1].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
		}
	}
	return retval;
}
template vector<unsigned int> read_array_two_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<int> read_array_two_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<float> read_array_two_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<double> read_array_two_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<string> read_array_two_level(ryml::ConstNodeRef & root, vector<string> const & path);


template<class T> vector<T> read_array_three_level(ryml::ConstNodeRef & root, vector<string> const & path)
{
	vector<T> retval;
	if ( path.size() == 3 ) {
		for ( int i = 0; i < root[path[0].c_str()][path[1].c_str()][path[2].c_str()].num_children(); i++ ) {
			if constexpr(std::is_same<T, unsigned int>::value) {
				unsigned int tmp = 0;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, int>::value) {
				int tmp = 0;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, float>::value) {
				float tmp = 0.;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, double>::value) {
				double tmp = 0.;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, string>::value) {
				string tmp = "";
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
		}
	}
	return retval;
}
template vector<unsigned int> read_array_three_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<int> read_array_three_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<float> read_array_three_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<double> read_array_three_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<string> read_array_three_level(ryml::ConstNodeRef & root, vector<string> const & path);


template<class T> vector<T> read_array_four_level(ryml::ConstNodeRef & root, vector<string> const & path)
{
	vector<T> retval;
	if ( path.size() == 4 ) {
		for ( int i = 0; i < root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()].num_children(); i++ ) {
			if constexpr(std::is_same<T, unsigned int>::value) {
				unsigned int tmp = 0;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, int>::value) {
				int tmp = 0.;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, float>::value) {
				float tmp = 0.;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, double>::value) {
				double tmp = 0.;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, string>::value) {
				string tmp = "";
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
		}
	}
	return retval;
}
template vector<unsigned int> read_array_four_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<int> read_array_four_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<float> read_array_four_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<double> read_array_four_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<string> read_array_four_level(ryml::ConstNodeRef & root, vector<string> const & path);


template<class T> vector<T> read_array_five_level(ryml::ConstNodeRef & root, vector<string> const & path)
{
	vector<T> retval;
	if ( path.size() == 5 ) {
		for ( int i = 0; i < root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][path[4].c_str()].num_children(); i++ ) {
			if constexpr(std::is_same<T, unsigned int>::value) {
				unsigned int tmp = 0;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][path[4].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, int>::value) {
				int tmp = 0.;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][path[4].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, float>::value) {
				float tmp = 0.;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][path[4].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, double>::value) {
				double tmp = 0.;
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][path[4].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
			else if constexpr(std::is_same<T, string>::value) {
				string tmp = "";
				root[path[0].c_str()][path[1].c_str()][path[2].c_str()][path[3].c_str()][path[4].c_str()][i] >> tmp;
				retval.push_back(tmp);
			}
		}
	}
	return retval;
}
template vector<unsigned int> read_array_five_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<int> read_array_five_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<float> read_array_five_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<double> read_array_five_level(ryml::ConstNodeRef & root, vector<string> const & path);
template vector<string> read_array_five_level(ryml::ConstNodeRef & root, vector<string> const & path);


template<typename T> vector<T> read_array(ryml::ConstNodeRef & root, const string flat_path)
{
	vector<T> retval;
	//stringstream test("entry1/numerics/snapshot_x");
	stringstream test(flat_path);
	//"entry1/discretization/grid/cell_dimensions/value")
	cout << "test " << test.str() << "\n";
	string segment;
	vector<string> levels; // = {"entry1", "numerics", "snapshot_x"};
	while(std::getline(test, segment, '/')) {
		levels.push_back(segment);
	}
	cout << "levels.size() " << levels.size() << "\n";
	switch(levels.size())
	{
		case 1: {
			cout << "\t" << "one_level" << "\n";
			retval = read_array_one_level<T>(root, levels);
			break;
		}
		case 2: {
			cout << "\t" << "two_level" << "\n";
			retval = read_array_two_level<T>(root, levels);
			break;
		}
		case 3: {
			cout << "\t" << "three_level" << "\n";
			retval = read_array_three_level<T>(root, levels);
			break;
		}
		case 4: {
			cout << "\t" << "four_level" << "\n";
			retval = read_array_four_level<T>(root, levels);
			break;
		}
		case 5: {
			cout << "\t" << "five_level" << "\n";
			retval = read_array_five_level<T>(root, levels);
			break;
		}
		default: {
			break;
		}
	}
	cout << "retval.size() " << retval.size() << "\n";
	return retval;
}
template vector<unsigned int> read_array(ryml::ConstNodeRef & root, const string flat_path);
template vector<int> read_array(ryml::ConstNodeRef & root, const string flat_path);
template vector<float> read_array(ryml::ConstNodeRef & root, const string flat_path);
template vector<double> read_array(ryml::ConstNodeRef & root, const string flat_path);
template vector<string> read_array(ryml::ConstNodeRef & root, const string flat_path);
*/

/*
int main(int argc, char** argv)
{
	if ( argc >= 1 ) {
		string fname = argv[1];
		cout << fname << "\n";
		string contents = file_get_contents<string>(fname.c_str());
		ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(contents)); // immutable (csubstr) overload
		ryml::ConstNodeRef root = tree.rootref();  // a const node reference

		//float f32 = 0.;
		//int i32 = 0;
		//int bar = 0;

		double bar = 0;
		string john_str;
		string bar_str;
		string clk_str;

		//root["f32"] >> f32;
		//cout << f32 << "\n";
		//root["i32"] >> i32;
		//cout << i32 << "\n";
		//cout << root["bar"].num_children() << ", " << root["bar"].num_siblings() << "\n";
		////for(ryml::ConstNodeRef const& child : root["bar"].siblings()) {

		//vector<int> bar_vec;
		vector<double> val_vec = ryml_wrapper_read_array<double>(root, "entry1/discretization/grid/extent");
		for ( auto it = val_vec.begin(); it != val_vec.end(); it++ ) {
			cout << "\t\t" << *it << "\n";
		}
		bar = ryml_wrapper_read_value<double>(root, "entry1/discretization/grid/cell_dimensions/value");
		cout << "cell_dimensions_value " << bar << "\n";

		bar_str = ryml_wrapper_read_value<string>(root, "entry1/discretization/grid/cell_dimensions/units");
		cout << "cell_dimensions_units " << bar_str << "\n";

		vector<string> str_vec = ryml_wrapper_read_array<string>(root, "string_array");
		for ( auto jt = str_vec.begin(); jt != str_vec.end(); jt++ ) {
			cout << "\t\t" << *jt << "\n";
		}

		//vector<int> bar_vec = read_yaml("bar");
		//bar_vec.insert(root["bar"].begin(), root["bar"].end());
		//for ( int i = 0; i < root["entry1"]["deformation"]["extent"]["value"].num_children(); i++ ) {

		//string entry = "entry1";
		//for ( int i = 0; i < root[entry.c_str()]["numerics"]["snapshot_x"].num_children(); i++ ) {
		//	//root["bar"]
		//	bar = 0;
		//	//root["entry1"]["deformation"]["extent"]["value"][i] >> bar;
		//	root[entry.c_str()]["numerics"]["snapshot_x"][i] >> bar;
		//	cout << bar << "\n";
		//	bar_vec.push_back(bar);
		//}
		//cout << bar_vec.size() << "\n";

		//root["john"] >> john_str;
		//// requires from_chars(std::string). see API doc.
		//root["bar"] >> ryml::key(bar_str);
		//root["clock"]["work"] >> clk_str;
		//// to deserialize the key, use the tag function ryml::key()
		//cout << john_str << "\n";
		//cout << bar_str << "\n";
		//cout << clk_str << "\n";
	}
	else {
		char yml_buf[] = "{foo: 1, bar: [2, 3], john: doe}";
		ryml::Tree tree = ryml::parse_in_place(yml_buf);
	}
	return 0;
}
*/


/*
void test_ryml()
{
	string fname = "NXmicrostructure_score_config.SimID.2.yaml";
	cout << fname << "\n";
	string contents = file_get_contents<string>(fname.c_str());
	ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(contents)); // immutable (csubstr) overload
	ryml::ConstNodeRef root = tree.rootref();  // a const node reference

	double bar = 0;
	string john_str;
	string bar_str;
	string clk_str;

	vector<double> val_vec = ryml_wrapper_read_array<double>(root, "entry1/discretization/grid/extent");
	for ( auto it = val_vec.begin(); it != val_vec.end(); it++ ) {
		cout << "\t\t" << *it << "\n";
	}
	bar = ryml_wrapper_read_value<double>(root, "entry1/discretization/grid/cell_dimensions/value");
	cout << "cell_dimensions_value " << bar << "\n";

	bar_str = ryml_wrapper_read_value<string>(root, "entry1/discretization/grid/cell_dimensions/units");
	cout << "cell_dimensions_units " << bar_str << "\n";

	vector<string> str_vec = ryml_wrapper_read_array<string>(root, "string_array");
	for ( auto jt = str_vec.begin(); jt != str_vec.end(); jt++ ) {
		cout << "\t\t" << *jt << "\n";
	}
	return;
}
*/
