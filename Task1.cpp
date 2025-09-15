// inverted_index.cpp
// C++17 - Build inverted index with postings as linked lists and compute tf-idf.
// Usage: ./inverted_index input.txt

#include <bits/stdc++.h>
using namespace std;

// --------------------- Simple stopword list ---------------------
const unordered_set<string> STOPWORDS = {
    "a","about","above","after","again","against","all","am","an","and","any","are","aren't","as","at",
    "be","because","been","before","being","below","between","both","but","by",
    "can't","cannot","could","couldn't",
    "did","didn't","do","does","doesn't","doing","don't","down","during",
    "each","few","for","from","further",
    "had","hadn't","has","hasn't","have","haven't","having","he","he'd","he'll","he's","her","here","here's","hers","herself","him","himself","his","how","how's",
    "i","i'd","i'll","i'm","i've","if","in","into","is","isn't","it","it's","its","itself",
    "let's",
    "me","more","most","mustn't","my","myself",
    "no","nor","not",
    "of","off","on","once","only","or","other","ought","our","ours","ourselves","out","over","own",
    "same","she","she'd","she'll","she's","should","shouldn't","so","some","such",
    "than","that","that's","the","their","theirs","them","themselves","then","there","there's","these","they","they'd","they'll","they're","they've","this","those","through","to","too",
    "under","until","up",
    "very",
    "was","wasn't","we","we'd","we'll","we're","we've","were","weren't","what","what's","when","when's","where","where's","which","while","who","who's","whom","why","why's","with","won't","would","wouldn't",
    "you","you'd","you'll","you're","you've","your","yours","yourself","yourselves"
};

// --------------------- Porter Stemmer (compact implementation) ---------------------
// This is a compact implementation of Porter's stemming algorithm adapted for clarity.
// It's not the full historical porter implementation but covers common cases.
struct PorterStemmer {
    static bool is_consonant(const string &w, int i) {
        char ch = w[i];
        if (string("aeiou").find(ch) != string::npos) return false;
        if (ch == 'y') {
            if (i == 0) return true;
            return !is_consonant(w, i-1);
        }
        return true;
    }
    // measure (m) = number of VC sequences
    static int measure(const string &w) {
        int n = w.size();
        int m = 0;
        int i = 0;
        while (i < n) {
            while (i < n && is_consonant(w, i)) ++i;
            if (i >= n) break;
            while (i < n && !is_consonant(w, i)) ++i;
            ++m;
        }
        return m;
    }
    static bool contains_vowel(const string &w) {
        for (int i=0;i<(int)w.size();++i) if (!is_consonant(w,i)) return true;
        return false;
    }
    static bool ends_with(const string &w, const string &s) {
        if (s.size() > w.size()) return false;
        return w.compare(w.size()-s.size(), s.size(), s) == 0;
    }
    static string replace_suffix(const string &w, const string &s, const string &rep) {
        if (!ends_with(w, s)) return w;
        return w.substr(0, w.size()-s.size()) + rep;
    }
    static bool cvc(const string &w) {
        int n = w.size();
        if (n < 3) return false;
        if (!is_consonant(w, n-1) || is_consonant(w, n-2) || !is_consonant(w, n-3)) return false;
        char ch = w[n-1];
        if (ch == 'w' || ch == 'x' || ch == 'y') return false;
        return true;
    }

    static string stem(string w) {
        if (w.size() <= 2) return w;
        // step 1a
        if (ends_with(w, "sses")) w = replace_suffix(w, "sses", "ss");
        else if (ends_with(w, "ies")) w = replace_suffix(w, "ies", "i");
        else if (ends_with(w, "ss")) {}
        else if (ends_with(w, "s")) w = replace_suffix(w, "s", "");

        // step 1b
        bool step1b_flag = false;
        if (ends_with(w, "eed")) {
            string stem = w.substr(0, w.size()-3);
            if (measure(stem) > 0) w = replace_suffix(w, "eed", "ee");
        } else {
            vector<string> suffixes = {"ed","ing"};
            for (auto &suf : suffixes) {
                if (ends_with(w, suf)) {
                    string stem = w.substr(0, w.size()-suf.size());
                    if (contains_vowel(stem)) {
                        w = stem;
                        step1b_flag = true;
                        break;
                    }
                }
            }
            if (step1b_flag) {
                if (ends_with(w, "at") || ends_with(w, "bl") || ends_with(w, "iz")) w += "e";
                else if (w.size() >= 2 && is_consonant(w, w.size()-1) && is_consonant(w, w.size()-2)
                         && w[w.size()-1] != 'l' && w[w.size()-1] != 's' && w[w.size()-1] != 'z') {
                    w = w.substr(0, w.size()-1);
                } else if (measure(w) == 1 && cvc(w)) {
                    w += "e";
                }
            }
        }

        // step 1c
        if (ends_with(w, "y")) {
            string stem = w.substr(0, w.size()-1);
            if (contains_vowel(stem)) w = replace_suffix(w, "y", "i");
        }

        // step 2 (selected suffixes)
        vector<pair<string,string>> step2 = {
            {"ational","ate"}, {"tional","tion"}, {"enci","ence"}, {"anci","ance"}, {"izer","ize"},
            {"abli","able"}, {"alli","al"}, {"entli","ent"}, {"eli","e"}, {"ousli","ous"},
            {"ization","ize"}, {"ation","ate"}, {"ator","ate"}, {"alism","al"}, {"iveness","ive"},
            {"fulness","ful"}, {"ousness","ous"}, {"aliti","al"}, {"iviti","ive"}, {"biliti","ble"}
        };
        for (auto &pr : step2) {
            if (ends_with(w, pr.first)) {
                string stem = w.substr(0, w.size()-pr.first.size());
                if (measure(stem) > 0) {
                    w = stem + pr.second;
                }
                break;
            }
        }

        // step 3
        vector<pair<string,string>> step3 = {
            {"icate","ic"}, {"ative",""}, {"alize","al"}, {"iciti","ic"}, {"ical","ic"},
            {"ful",""}, {"ness",""}
        };
        for (auto &pr : step3) {
            if (ends_with(w, pr.first)) {
                string stem = w.substr(0, w.size()-pr.first.size());
                if (measure(stem) > 0) {
                    w = stem + pr.second;
                }
                break;
            }
        }

        // step 4
        vector<string> step4 = {
            "al","ance","ence","er","ic","able","ible","ant","ement","ment","ent","ion","ou","ism","ate","iti","ous","ive","ize"
        };
        for (auto &suf : step4) {
            if (ends_with(w, suf)) {
                string stem = w.substr(0, w.size()-suf.size());
                if (measure(stem) > 1) {
                    if (suf == "ion") {
                        if (!stem.empty() && (stem.back() == 's' || stem.back() == 't')) {
                            w = stem;
                        }
                    } else {
                        w = stem;
                    }
                }
                break;
            }
        }

        // step 5a
        if (ends_with(w, "e")) {
            string stem = w.substr(0, w.size()-1);
            int m = measure(stem);
            if (m > 1 || (m == 1 && !cvc(stem))) w = stem;
        }

        // step 5b
        if (measure(w) > 1 && w.size()>=2 && w[w.size()-1] == w[w.size()-2] && w.back() == 'l') {
            w = w.substr(0, w.size()-1);
        }

        return w;
    }
};

// --------------------- Data structures for inverted index ---------------------
struct Posting {
    int docid;
    int freq;         // frequency of term in this doc (after preprocessing)
    double tf;        // term frequency (freq / total_tokens_in_doc)
    double tfidf;     // tf * idf
    Posting *next;
    Posting(int d=0, int f=0): docid(d), freq(f), tf(0.0), tfidf(0.0), next(nullptr) {}
};

struct PostingsList {
    Posting *head = nullptr;
    int docFrequency() const {
        int cnt = 0;
        for (Posting* p = head; p; p = p->next) ++cnt;
        return cnt;
    }
    // insert or update posting; keep list ordered by increasing docid
    void add_or_inc(int docid, int inc=1) {
        if (!head) {
            head = new Posting(docid, inc);
            return;
        }
        // if should be new head
        if (docid < head->docid) {
            Posting* n = new Posting(docid, inc);
            n->next = head;
            head = n;
            return;
        }
        Posting* cur = head; Posting* prev = nullptr;
        while (cur && cur->docid < docid) {
            prev = cur; cur = cur->next;
        }
        if (cur && cur->docid == docid) {
            cur->freq += inc;
        } else {
            Posting* n = new Posting(docid, inc);
            if (prev) {
                prev->next = n;
                n->next = cur;
            }
        }
    }
};

// --------------------- Utility preprocessing ---------------------
static inline string to_lower(const string &s) {
    string out; out.reserve(s.size());
    for (char c: s) out.push_back((char)tolower((unsigned char)c));
    return out;
}

string remove_special_keep_alnum_space(const string &s) {
    string out; out.reserve(s.size());
    for (char c: s) {
        if (isalnum((unsigned char)c) || isspace((unsigned char)c)) out.push_back(c);
        // otherwise skip
    }
    return out;
}

string normalize_spaces(const string &s) {
    string out;
    bool in_space = false;
    for (char c: s) {
        if (isspace((unsigned char)c)) {
            if (!in_space) {
                out.push_back(' ');
                in_space = true;
            }
        } else {
            out.push_back(c);
            in_space = false;
        }
    }
    // trim
    if (!out.empty() && out.front() == ' ') out.erase(out.begin());
    if (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

vector<string> whitespace_tokenize(const string &s) {
    vector<string> tokens;
    string token;
    stringstream ss(s);
    while (ss >> token) tokens.push_back(token);
    return tokens;
}

// --------------------- Main pipeline ---------------------
int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " input.txt\n";
        return 1;
    }

    string input_path = argv[1];
    ifstream infile(input_path);
    if (!infile) {
        cerr << "Cannot open input file: " << input_path << "\n";
        return 1;
    }

    unordered_map<string, PostingsList> inverted; // term -> postings linked list
    unordered_map<int, int> doc_total_tokens;     // docid -> total tokens after preprocessing
    int total_docs = 0;
    string line;

    // Read each line -> parse docID and document text
    while (getline(infile, line)) {
        if (line.empty()) continue;
        // Allow either "docid<TAB>text" or "docid space text"
        stringstream ss(line);
        string docid_str;
        if (!(ss >> docid_str)) continue;
        int docid;
        try { docid = stoi(docid_str); }
        catch (...) {
            // If the docid isn't first token, try parse line by splitting on tab
            size_t tabpos = line.find('\t');
            if (tabpos == string::npos) continue;
            docid = stoi(line.substr(0, tabpos));
            // set remainder appropriately
            ss.clear();
            ss.str(line.substr(tabpos+1));
        }
        string rest;
        getline(ss, rest); // the rest of line (may start with space)
        if (rest.size()>0 && rest[0]==' ') rest.erase(rest.begin());

        // Preprocessing pipeline
        // a) lowercase
        string doc = to_lower(rest);
        // b) remove special chars (only keep alnum and spaces)
        doc = remove_special_keep_alnum_space(doc);
        // c) normalize spaces
        doc = normalize_spaces(doc);
        if (doc.empty()) {
            // still count doc with zero tokens perhaps
            doc_total_tokens[docid] = 0;
            ++total_docs;
            continue;
        }
        // d) tokenize by whitespace
        vector<string> tokens = whitespace_tokenize(doc);
        // e) remove stopwords
        vector<string> kept;
        kept.reserve(tokens.size());
        for (auto &t: tokens) if (STOPWORDS.find(t) == STOPWORDS.end()) kept.push_back(t);
        // f) Porter stem
        for (auto &t: kept) t = PorterStemmer::stem(t);

        // update doc total tokens (after preprocessing)
        doc_total_tokens[docid] = (int)kept.size();

        // if doc has zero tokens after preprocessing, still count doc
        ++total_docs;

        // build term frequencies for this doc (so we can insert frequency)
        unordered_map<string,int> tf;
        for (auto &t: kept) ++tf[t];

        // insert into inverted index; postings must be ordered by increasing docid
        for (auto &pr: tf) {
            const string &term = pr.first;
            int freq = pr.second;
            // add_or_inc will add posting or increase freq
            inverted[term].add_or_inc(docid, freq);
        }
    }

    infile.close();

    // Now compute tf and tf-idf for each posting.
    // idf = total_num_docs / length_of_postings_list
    for (auto &entry : inverted) {
        const string &term = entry.first;
        PostingsList &plist = entry.second;
        int df = plist.docFrequency();
        double idf = 0.0;
        if (df > 0) idf = (double)total_docs / (double)df;

        // iterate postings and compute tf and tfidf
        for (Posting* p = plist.head; p; p = p->next) {
            int docid = p->docid;
            int freq = p->freq;
            int total_tokens_in_doc = doc_total_tokens.count(docid) ? doc_total_tokens[docid] : 0;
            double tf = 0.0;
            if (total_tokens_in_doc > 0) tf = (double)freq / (double)total_tokens_in_doc;
            p->tf = tf;
            p->tfidf = tf * idf;
        }
    }

    // Sample output: print inverted index
    // Sorted terms for deterministic output
    vector<string> terms;
    terms.reserve(inverted.size());
    for (auto &e: inverted) terms.push_back(e.first);
    sort(terms.begin(), terms.end());
    ofstream fout("task1.output.txt");
    cout << "Total docs processed: " << total_docs << "\n";
    fout << "Total docs processed: " << total_docs << "\n";

    cout << "Vocabulary size: " << terms.size() << "\n\n";
    fout << "Vocabulary size: " << terms.size() << "\n\n";

    for (auto &term : terms) {
        cout << "TERM: " << term << "  DF=" << inverted[term].docFrequency() << "\n";
        fout << "TERM: " << term << "  DF=" << inverted[term].docFrequency() << "\n";
        
        cout << "Postings -> [docid, freq, tf, tf-idf]\n";
        fout << "Postings -> [docid, freq, tf, tf-idf]\n";
        for (Posting* p = inverted[term].head; p; p = p->next) {
            cout.setf(std::ios::fixed); cout<<setprecision(6);
            cout << "  [" << p->docid << ", " << p->freq << ", " << p->tf << ", " << p->tfidf << "]\n"; 

            fout.setf(std::ios::fixed);
            fout<<setprecision(6);
            fout << "  [" << p->docid << ", " << p->freq << ", " << p->tf << ", " << p->tfidf << "]\n";
        }
        cout << "\n";
        fout << "\n";
    }
    fout.close();
    return 0;
}
