import re
from collections import Counter

# load file
file_path = "nytimes_paragraphs_selenium.txt"
with open(file_path, "r", encoding="utf-8") as f:
    lines = [line.strip() for line in f if line.strip()]

# extract the actual text (ignoring DocIDs)
texts = []
for line in lines:
    if '\t' in line:
        docid, text = line.split('\t', 1)
    else:
        parts = line.split(maxsplit=1)
        if len(parts) == 2:
            docid, text = parts
        else:
            continue
    texts.append(text)

# number of sentences = number of lines here
num_sentences = len(texts)

# characters
num_characters = sum(len(t) for t in texts)

# tokenize by whitespace and punctuation
token_pattern = re.compile(r"\b\w+\b")
all_tokens = []
for t in texts:
    tokens = token_pattern.findall(t.lower())
    all_tokens.extend(tokens)

num_tokens = len(all_tokens)

# average tokens per sentence
avg_tokens_per_sentence = num_tokens / num_sentences if num_sentences else 0

# most common tokens
token_counts = Counter(all_tokens)
most_common_tokens = token_counts.most_common(20)

# very simple "topic" approximation: top bigrams
bigrams = Counter(zip(all_tokens, all_tokens[1:]))
most_common_bigrams = bigrams.most_common(20)

print("Number of sentences:", num_sentences)
print("Number of characters:", num_characters)
print("Number of tokens:", num_tokens)
print("Average tokens per sentence:", avg_tokens_per_sentence)
print("\nTop 20 tokens:")
for tok, freq in most_common_tokens:
    print(f"{tok}: {freq}")

print("\nTop 20 bigrams (rough topic indicators):")
for (w1, w2), freq in most_common_bigrams:
    print(f"{w1} {w2}: {freq}")
