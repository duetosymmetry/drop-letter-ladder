#include <iostream>
#include <fstream>

#include <cctype>
#include <algorithm>

#include "cxxopts.hpp"

std::vector<std::string> allDrops(const std::string& s)
{
  size_t n = s.length();
  std::vector<std::string> ret(n);

  for(size_t i=0; i<n; i++) {
    ret[i] = s.substr(0, i) + s.substr(i+1, std::string::npos);
  };

  return ret;
};



int main(int argc, char* argv[]) {
  try {

    std::string dictPath;

    cxxopts::Options options(argv[0], "Find ladders of words formed by letter-dropping");

    options.add_options()
      ("d,dict", "Dictionary file path", cxxopts::value<std::string>(dictPath)
       ->default_value("/usr/share/dict/words"), "FILE")
      ("help", "Print this help message")
      ;

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help({"", "Group"}) << std::endl;
      exit(0);
    }

    ////////////////////////////////////////////////////////
    
    // Open dictionary file
    std::ifstream dictFile;

    dictFile.open(dictPath);
    if (! dictFile.good() ) {
      std::cerr << "Can't read from dictionary file '"
                << dictPath << "'" << std::endl;
      exit(1);
    };

    // Go through the dictionary file in one pass
    
    std::vector<std::string> dict;
    std::string line;

    while (std::getline(dictFile, line)) {
      // downcase all the characters in line
      std::transform(line.begin(), line.end(),
                     line.begin(), tolower);

      dict.push_back(line);

    }

    // All done!
    dictFile.close();

    // Make sure it's sorted and unique
    sort(dict.begin(), dict.end());

    {
    auto it = unique(dict.begin(), dict.end());

    dict.resize( distance(dict.begin(), it) );
    };
    
    ////////////////////////////////////////////////////////

    // Brute force approach, find all the drop-pairs

    std::vector< std::tuple< decltype(dict)::iterator,
                             decltype(dict)::iterator > > dropPairs;

    for ( auto it = dict.begin(); it < dict.end(); it++ ) {
      auto wordDrops = allDrops(*it);

      for (auto d : wordDrops) {
        auto bounds = equal_range(dict.begin(), dict.end(), d);
        if (bounds.second > bounds.first)
          dropPairs.push_back( make_tuple(it, bounds.first) );
      };
    };

    std::cout << "Length " << dropPairs.size() << std::endl;

    return 0;

    for (auto p : dropPairs ) {
      std::cout << *(std::get<0>(p)) << "->" << *(std::get<1>(p)) << std::endl;
    };

    ////////////////////////////////////////////////////////

  } catch (const cxxopts::OptionException& e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }

  return 0;
}
