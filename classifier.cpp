#include <iostream>
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <utility>
#include <cmath>
#include "csvstream.hpp"   
using namespace std;

class classifier {
    public:
        //REQUIRES: a valid label, me taking a nap
        //MODIFIES: nothing
        //EFFECTS: returns the log prior probability of a label C
        double logPrior(const string &label) {
            return log(static_cast<double>(postsWithLabel[label]) / totalPosts);
        }

        //REQUIRES: valid word and label
        //MODIFIES: nothing
        //EFFECTS: returns the log likelihood of a word w given a label C
        double logLikelihood(const string &word, const string &label) {
            if (postsContainingWord.count(word) == 0) {
                return log(1.0 / static_cast<double>(totalPosts));
            };

            if (postsWithLabelContainsWord.find({label, word}) 
            != postsWithLabelContainsWord.end() 
            && postsWithLabelContainsWord[{label, word}] > 0) {

            return log(static_cast<double>(postsWithLabelContainsWord[{label, word}])
             / static_cast<double>(postsWithLabel[label]));

            } else {

                return log(static_cast<double>(postsContainingWord[word]) /
                   static_cast<double>(totalPosts));

            }
        }

        set<string> unique_words(const string &str) {
            istringstream source(str);
            set<string> words;
            string word;
            while (source >> word) {
                words.insert(word);
            }
            return words;
        }

        //REQUIRES: valid file and filename
        //MODIFIES: private variables
        //EFFECTS: trains the classifier on the data in filename csv
        void train(const string &filename, bool log) {
            csvstream csvin(filename);
            map<string, string> row;

            if (log == true) {
                cout << "training data:" << endl;
            }

            while (csvin >> row) {
                string label = row["tag"];
                string content = row["content"];

                if (log == true) {
                    cout << "  label = " << label 
                    << ", content = " << content << endl;
                }

                set<string> words = unique_words(content);
                //le tiebreker

                ++totalPosts;
                postsWithLabel[label]++;
                vocabulary.insert(words.begin(), words.end());

                for (string word : words) {
                    postsContainingWord[word]++;
                    postsWithLabelContainsWord[{label, word}]++;
                }
            }

            vocabSize = vocabulary.size();

            if (log == true) {
                cout << "trained on " << totalPosts << " examples" << endl;
                cout << "vocabulary size = " << vocabSize << "\n" << endl;

            cout << "classes:\n";
            for (auto label : postsWithLabel) {
                cout << "  " << label.first << ", " << label.second <<
                 " examples, log-prior = " << logPrior(label.first) << "\n";
            }
            cout << "classifier parameters:\n";
            for (auto label : postsWithLabel) {
                for (string word : vocabulary) { 
                    if (postsWithLabelContainsWord.count({label.first, word}) > 0) {

                        cout << "  " << label.first << ":" << word << ", count = " 
                        << postsWithLabelContainsWord[{label.first, word}] 
                        << ", log-likelihood = " 
                        << logLikelihood(word, label.first) << "\n";

                    }
                }
            }

            cout << endl;
            }
        }

        //REQUIRES: valid content, label and prob variable from the calling function
        //MODIFIES: label and prob
        //EFFECTS: changes label prob to whatever has the highest predicted probability 
        void predict(std::string &label, double &prob, const std::string &content) {
            set<string> words = unique_words(content);

            bool first = true; 

            for (auto labels : postsWithLabel) {
                string current = labels.first;

                double logProb = logPrior(current);

                for (string word : words) {
                    logProb += logLikelihood(word, current);
                }

                if (first || (logProb > prob) || (logProb == prob && current < label)) {
                    first = false;
                    prob = logProb;
                    label = current;
                }
            }
        }

        //REQUIRES: valid file and filename
        //MODIFIES: nothing
        //EFFECTS: tests on input data and prints results
        void test(const string &filename) {
            csvstream csvin(filename);
            map<string, string> row;

            cout << "trained on " << totalPosts << " examples" << "\n" << endl;
            cout << "test data:" << endl;

            string correctLabel;
            double calculatedProb;
            int correctCount = 0;
            int tried = 0;

            while (csvin >> row) {
                string label = row["tag"];
                string content = row["content"];

                predict(correctLabel, calculatedProb, content);
                ++tried;

                if (correctLabel == label) {
                    ++correctCount;
                }

                cout << "  correct = " << label << ", predicted = " << correctLabel 
                << ", log-probability score = " << calculatedProb << "\n";
                cout << "  content = " << content << "\n" << endl;
            }

            cout << "performance: " << correctCount << " / " 
            << tried << " posts predicted correctly" << endl;
    }
    
    private:
        int totalPosts = 0;
        int vocabSize = 0;

        set<string> vocabulary;
        map<string, int> postsContainingWord; 
        map<string, int> postsWithLabel;
        map<pair<string, string>, int> postsWithLabelContainsWord;
};

int main(int argc, char* argv[]) {
    cout.precision(3);

    if (argc != 2 && argc != 3) {
        cout << "Usage: classifier.exe TRAIN_FILE [TEST_FILE]" << endl;
        return 1;
    }

    string fileToTrain = argv[1];
    classifier cls;

    if (argc == 2) {
        try {
            cls.train(fileToTrain, true);
        } catch (const csvstream_exception &e) {
            cout << "Error opening file: " << fileToTrain << endl;
            return 2;
        }
    }

    if (argc == 3) {
        string fileToTest = argv[2];
        try {
            cls.train(fileToTrain, false);
        } catch (const csvstream_exception &e) {
            cout << "Error opening file: " << fileToTrain << endl;
            return 3;
        }

        try {
            cls.test(fileToTest);
        } catch (const csvstream_exception &e) {
            cout << "Error opening file: " << fileToTest << endl;
            return 4;
        }
    }
}