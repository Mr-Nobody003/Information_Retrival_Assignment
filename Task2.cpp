#include <bits/stdc++.h>
using namespace std;

/* ---------------- stopwords ---------------- */
const unordered_set<string> STOPWORDS = {
    "a","about","above","after","again","against","all","am","an","and","any","are","aren't","as","at",
    "be","because","been","before","being","below","between","both","but","by","can't","cannot","could","couldn't",
    "did","didn't","do","does","doesn't","doing","don't","down","during","each","few","for","from","further",
    "had","hadn't","has","hasn't","have","haven't","having","he","he'd","he'll","he's","her","here","here's","hers","herself","him","himself","his","how","how's",
    "i","i'd","i'll","i'm","i've","if","in","into","is","isn't","it","it's","its","itself","let's","me","more","most","mustn't","my","myself",
    "no","nor","not","of","off","on","once","only","or","other","ought","our","ours","ourselves","out","over","own","same","she","she'd","she'll","she's",
    "should","shouldn't","so","some","such","than","that","that's","the","their","theirs","them","themselves","then","there","there's","these","they","they'd",
    "they'll","they're","they've","this","those","through","to","too","under","until","up","very","was","wasn't","we","we'd","we'll","we're","we've","were",
    "weren't","what","what's","when","when's","where","where's","which","while","who","who's","whom","why","why's","with","won't","would","wouldn't",
    "you","you'd","you'll","you're","you've","your","yours","yourself","yourselves"
};

/* ---------------- Porter stemmer (simplified) ---------------- */
struct PorterStemmer {
    static bool is_consonant(const string &w, int i) {
        char ch=w[i];
        if (string("aeiou").find(ch)!=string::npos) return false;
        if (ch=='y') return i==0?true:!is_consonant(w,i-1);
        return true;
    }
    static int measure(const string &w) {
        int n=w.size(),m=0,i=0;
        while (i<n) {
            while (i<n && is_consonant(w,i)) i++;
            if (i>=n) break;
            while (i<n && !is_consonant(w,i)) i++;
            m++;
        }
        return m;
    }
    static bool contains_vowel(const string &w) {
        for (int i=0;i<(int)w.size();++i) if (!is_consonant(w,i)) return true;
        return false;
    }
    static bool ends_with(const string &w,const string&s){
        if(s.size()>w.size())return false;
        return w.compare(w.size()-s.size(),s.size(),s)==0;
    }
    static string replace_suffix(const string&w,const string&s,const string&rep){
        if(!ends_with(w,s))return w;
        return w.substr(0,w.size()-s.size())+rep;
    }
    static bool cvc(const string&w){
        int n=w.size();
        if(n<3)return false;
        if(!is_consonant(w,n-1)||is_consonant(w,n-2)||!is_consonant(w,n-3))return false;
        char ch=w[n-1];
        if(ch=='w'||ch=='x'||ch=='y')return false;
        return true;
    }
    static string stem(string w){
        if(w.size()<=2)return w;
        if(ends_with(w,"sses"))w=replace_suffix(w,"sses","ss");
        else if(ends_with(w,"ies"))w=replace_suffix(w,"ies","i");
        else if(ends_with(w,"ss")){}
        else if(ends_with(w,"s"))w=replace_suffix(w,"s","");
        bool flag=false;
        if(ends_with(w,"eed")){
            string stem=w.substr(0,w.size()-3);
            if(measure(stem)>0)w=replace_suffix(w,"eed","ee");
        }else{
            for(auto&suf:{"ed","ing"}){
                if(ends_with(w,suf)){
                    string stem=w.substr(0,w.size()-string(suf).size());
                    if(contains_vowel(stem)){w=stem;flag=true;break;}
                }
            }
            if(flag){
                if(ends_with(w,"at")||ends_with(w,"bl")||ends_with(w,"iz"))w+="e";
                else if(w.size()>=2&&is_consonant(w,w.size()-1)&&is_consonant(w,w.size()-2)
                        &&w.back()!='l'&&w.back()!='s'&&w.back()!='z')w=w.substr(0,w.size()-1);
                else if(measure(w)==1&&cvc(w))w+="e";
            }
        }
        if(ends_with(w,"y")){
            string stem=w.substr(0,w.size()-1);
            if(contains_vowel(stem))w=replace_suffix(w,"y","i");
        }
        return w;
    }
};

/* ---------------- Inverted index structures ---------------- */
struct Posting {
    int docid,freq; double tf,tfidf; Posting*next;
    Posting(int d=0,int f=0):docid(d),freq(f),tf(0),tfidf(0),next(nullptr){}
};
struct PostingsList {
    Posting*head=nullptr;
    int docFrequency()const{int c=0;for(Posting*p=head;p;p=p->next)c++;return c;}
    void add_or_inc(int docid,int inc=1){
        if(!head){head=new Posting(docid,inc);return;}
        if(docid<head->docid){
            Posting*n=new Posting(docid,inc);n->next=head;head=n;return;}
        Posting*cur=head,*prev=nullptr;
        while(cur&&cur->docid<docid){prev=cur;cur=cur->next;}
        if(cur&&cur->docid==docid)cur->freq+=inc;
        else{
            Posting*n=new Posting(docid,inc);
            if(prev){prev->next=n;n->next=cur;}
        }
    }
};

/* ---------------- Utility functions ---------------- */
string to_lower(const string&s){string out;out.reserve(s.size());for(char c:s)out.push_back(tolower((unsigned char)c));return out;}
string remove_special_keep_alnum_space(const string&s){string out;out.reserve(s.size());for(char c:s)if(isalnum((unsigned char)c)||isspace((unsigned char)c))out.push_back(c);return out;}
string normalize_spaces(const string&s){string out;bool in_space=false;for(char c:s){if(isspace((unsigned char)c)){if(!in_space){out.push_back(' ');in_space=true;}}else{out.push_back(c);in_space=false;}}if(!out.empty()&&out.front()==' ')out.erase(out.begin());if(!out.empty()&&out.back()==' ')out.pop_back();return out;}
vector<string> whitespace_tokenize(const string&s){vector<string>tokens;string t;stringstream ss(s);while(ss>>t)tokens.push_back(t);return tokens;}

/* ---------------- Query processing helpers ---------------- */
vector<string> preprocess_query(const string&q){
    string s=to_lower(q);
    s=remove_special_keep_alnum_space(s);
    s=normalize_spaces(s);
    vector<string>toks=whitespace_tokenize(s);
    vector<string>kept;
    for(auto&t:toks)if(STOPWORDS.find(t)==STOPWORDS.end())kept.push_back(t);
    for(auto&t:kept)t=PorterStemmer::stem(t);
    return kept;
}
vector<int> get_postings(const unordered_map<string,PostingsList>&inverted,const string&term){
    vector<int>docs;auto it=inverted.find(term);
    if(it==inverted.end())return docs;
    for(Posting*p=it->second.head;p;p=p->next)docs.push_back(p->docid);
    sort(docs.begin(),docs.end());
    return docs;
}
vector<int> boolean_and(const unordered_map<string,PostingsList>&inverted,const vector<string>&terms){
    if(terms.empty())return{};
    vector<int>res=get_postings(inverted,terms[0]);
    for(size_t i=1;i<terms.size();++i){
        vector<int>pl=get_postings(inverted,terms[i]);
        vector<int>inter;size_t a=0,b=0;
        while(a<res.size()&&b<pl.size()){
            if(res[a]==pl[b]){inter.push_back(res[a]);++a;++b;}
            else if(res[a]<pl[b])++a;else++b;
        }
        res.swap(inter);
        if(res.empty())break;
    }
    return res;
}

/* ---------------- main ---------------- */
int main(int argc,char**argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(argc<2){cerr<<"Usage: "<<argv[0]<<" input.txt\n";return 1;}
    string input_path=argv[1];
    ifstream infile(input_path);
    if(!infile){cerr<<"Cannot open input file\n";return 1;}

    unordered_map<string,PostingsList>inverted;
    unordered_map<int,int>doc_total_tokens;
    int total_docs=0;string line;
    while(getline(infile,line)){
        if(line.empty())continue;
        stringstream ss(line);string docid_str; if(!(ss>>docid_str))continue;
        int docid;try{docid=stoi(docid_str);}catch(...){
            size_t tabpos=line.find('\t');if(tabpos==string::npos)continue;
            docid=stoi(line.substr(0,tabpos));ss.clear();ss.str(line.substr(tabpos+1));
        }
        string rest;getline(ss,rest);if(!rest.empty()&&rest[0]==' ')rest.erase(rest.begin());
        string doc=to_lower(rest);
        doc=remove_special_keep_alnum_space(doc);
        doc=normalize_spaces(doc);
        if(doc.empty()){doc_total_tokens[docid]=0;++total_docs;continue;}
        vector<string>tokens=whitespace_tokenize(doc);
        vector<string>kept;for(auto&t:tokens)if(STOPWORDS.find(t)==STOPWORDS.end())kept.push_back(t);
        for(auto&t:kept)t=PorterStemmer::stem(t);
        doc_total_tokens[docid]=kept.size();
        ++total_docs;
        unordered_map<string,int>tf;for(auto&t:kept)++tf[t];
        for(auto&pr:tf)inverted[pr.first].add_or_inc(docid,pr.second);
    }
    infile.close();
    // compute tf-idf
    for(auto&entry:inverted){
        PostingsList&pl=entry.second;int df=pl.docFrequency();double idf=(df>0)?(double)total_docs/df:0;
        for(Posting*p=pl.head;p;p=p->next){
            int total_tokens=doc_total_tokens.count(p->docid)?doc_total_tokens[p->docid]:0;
            double tf=(total_tokens>0)?(double)p->freq/total_tokens:0;
            p->tf=tf;p->tfidf=tf*idf;
        }
    }

    // === Task 2: Boolean queries ===
    vector<string>queries={
        "new york",
        "america",
        "white house",
        "donald trump",
        "joe biden",
        "global warming effect",
        "election result update",
        "stock market crash",
        "covid vaccine research",
        "sports team wins"
    };
    ofstream qout("task2.output.txt");
    for(size_t i=0;i<queries.size();++i){
        vector<string>qterms=preprocess_query(queries[i]);
        qout<<"Query "<<(i+1)<<": \""<<queries[i]<<"\"\nProcessed terms: ";
        for(auto&t:qterms)qout<<t<<" ";qout<<"\n";
        for(auto&t:qterms){
            vector<int>pl=get_postings(inverted,t);
            qout<<"Postings for ["<<t<<"]: ";for(int d:pl)qout<<d<<" ";qout<<"\n";
        }
        vector<int>anddocs=boolean_and(inverted,qterms);
        qout<<"AND-matched docs: ";for(int d:anddocs)qout<<d<<" ";qout<<"\n\n";
    }
    qout.close();
    cerr<<"Task2 done. Output in task2.output.txt\n";
    return 0;
}
