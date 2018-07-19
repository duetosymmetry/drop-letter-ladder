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

struct wordGraphNode {
  wordGraphNode( const std::string s ) : word(std::move(s)) {} ;

  std::string word;
  std::vector< const wordGraphNode* > inEdges, outEdges;

  bool isRoot() const noexcept { return inEdges.empty(); };
  bool isLeaf() const noexcept { return outEdges.empty(); };
};

struct wordForestOfTrees
  : public std::vector< wordGraphNode >
{
  std::vector< const wordGraphNode * > roots() const
  {
    std::vector< const wordGraphNode * > ret;

    for ( auto word = begin(); word < end(); word++)
      if (word->isRoot())
        ret.push_back(&*word);

    return ret;
  };

  std::vector< const wordGraphNode * > nonTrivRoots() const
  {
    std::vector< const wordGraphNode * > ret;

    for ( auto word = begin(); word < end(); word++)
      if (word->isRoot() && !(word->isLeaf()))
        ret.push_back(&*word);

    return ret;
  };
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
    wordForestOfTrees dict2;
    std::string line;

    while (std::getline(dictFile, line)) {
      // downcase all the characters in line
      std::transform(line.begin(), line.end(),
                     line.begin(), tolower);

      dict.push_back(line);
      dict2.push_back(line);

    }

    // All done!
    dictFile.close();

    // Make sure it's sorted and unique
    sort(dict.begin(), dict.end());
    sort(dict2.begin(), dict2.end(),
         [](const wordGraphNode & a , const wordGraphNode & b) {
           return a.word < b.word;
         });

    std::cout << "Length " << dict2.size() << std::endl;

    {
    auto last = unique(dict.begin(), dict.end());

    dict.erase( last, dict.end() );
    };

    std::cout << "Length " << dict.size() << std::endl;

    {
    auto last = unique(dict2.begin(), dict2.end(),
                       [](const wordGraphNode & a , const wordGraphNode & b) {
                         return a.word == b.word;
                       });

    dict2.erase( last, dict2.end() );
    };

    std::cout << "Length " << dict2.size() << std::endl;

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

    ////////////////////////////////////////////////////////

    // Brute force approach, find all the drop-pairs

    for ( auto it = dict2.begin(); it < dict2.end(); it++ ) {
      auto wordDrops = allDrops(it->word);

      for (auto d : wordDrops) {
        auto bounds = equal_range(dict2.begin(), dict2.end(), d,
                                  [](const wordGraphNode & a,
                                     const wordGraphNode & b) {
                                    return a.word < b.word;
                                  });
        if (bounds.second > bounds.first) {
          it->outEdges.push_back(&*bounds.first);
          bounds.first->inEdges.push_back(&*it);
        };
      };
    };

    ////////////////////////////////////////////////////////

    auto roots = dict2.nonTrivRoots();

    std::cout << roots.size() << std::endl;

    for ( auto root : roots ) {
      std::cout << root->word << std::endl;
    };

    return 0;

    ////////////////////////////////////////////////////////

  } catch (const cxxopts::OptionException& e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }

  return 0;
}
