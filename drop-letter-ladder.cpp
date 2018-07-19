#include <iostream>
#include <fstream>

#include <cctype>

#include <stack>

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

struct wordGraphNode
  : public std::string
{

  std::vector< const wordGraphNode* > inEdges, outEdges;

  bool isRoot() const noexcept { return inEdges.empty(); };
  bool isLeaf() const noexcept { return outEdges.empty(); };
};

struct wordChain
  : public std::vector< const wordGraphNode * >
{
  bool complete() const { return (*(end()-1))->isLeaf(); };
  bool incomplete() const { return !complete(); };

  std::vector< wordChain > singleWordExtensions() const {
    std::vector< wordChain > ret;

    if (complete()) return ret;

    auto lastWord = *(end()-1);

    for ( auto newLastWord : lastWord->outEdges ) {
      wordChain myCopy(*this);
      // std::copy(begin(), end(), myCopy.begin());
      myCopy.push_back(newLastWord);
      ret.push_back(myCopy);
    };

    return ret;

  };
};

std::ostream & operator<<(std::ostream &os, const wordChain & chain)
{
  for (auto it = chain.begin(); it < chain.end(); it++) {
    os << **it;
    if (it != chain.end()-1)
      os << "->";
  };

  return os;
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

    wordForestOfTrees dict;
    std::string line;

    while (std::getline(dictFile, line)) {
      // downcase all the characters in line
      std::transform(line.begin(), line.end(),
                     line.begin(), tolower);

      dict.push_back(wordGraphNode{line});

    }

    // All done!
    dictFile.close();

    // Make sure it's sorted and unique
    sort(dict.begin(), dict.end());

    // std::cout << "Length " << dict.size() << std::endl;

    {
    auto last = unique(dict.begin(), dict.end());

    dict.erase( last, dict.end() );
    };

    // std::cout << "Length " << dict.size() << std::endl;

    ////////////////////////////////////////////////////////
    // Brute force approach, find all the drop-pairs

    for ( auto it = dict.begin(); it < dict.end(); it++ ) {
      auto wordDrops = allDrops(*it);

      for (auto d : wordDrops) {
        auto bounds = equal_range(dict.begin(), dict.end(), d);
        if (bounds.second > bounds.first) {
          it->outEdges.push_back(&*bounds.first);
          bounds.first->inEdges.push_back(&*it);
        };
      };
    };

    ////////////////////////////////////////////////////////
    // Do a depth-first search to collect all the word-chains. Use a stack.

    std::vector< wordChain > completed;
    std::stack< wordChain > incompleted;

    auto roots = dict.nonTrivRoots();

    for ( auto root : roots ) {
      wordChain newChain;
      newChain.push_back(root);
      incompleted.push(newChain);
    }

    while (!incompleted.empty()) {
      auto chain = incompleted.top();
      incompleted.pop();

      // std::cout << "popped " << chain << std::endl;

      if (chain.complete()) {
        completed.push_back(chain);
      } else {
        auto extensions = chain.singleWordExtensions();
        for ( auto extension : extensions ) {
          // std::cout << "pushing " << extension << std::endl;
          incompleted.push(extension);
        };
      };
    };

    std::cout << completed.size() << " chains" << std::endl;

    ////////////////////////////////////////////////////////
    // Sort by length of chain

    std::sort( completed.begin(), completed.end(),
               []( const wordChain & a, const wordChain & b ) {
                 return a.size() > b.size();
               });

    for ( auto chain : completed )
      std::cout << chain << std::endl;

    // for ( auto root : roots ) {
    //   std::cout << *root << " " << *(root->outEdges[0]) << std::endl;
    // };

    return 0;

    ////////////////////////////////////////////////////////

  } catch (const cxxopts::OptionException& e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }

  return 0;
}
