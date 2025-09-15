from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.options import Options
import nltk
import time

# Download both resources
nltk.download('punkt')
nltk.download('punkt_tab')

chrome_options = Options()
# chrome_options.add_argument("--headless")
chrome_options.add_argument("--disable-gpu")
chrome_options.add_argument("--no-sandbox")
chrome_options.add_argument("--log-level=3")  # suppress logs

driver = webdriver.Chrome(options=chrome_options)

try:
    driver.get("https://www.nytimes.com/")
    time.sleep(5)

    paragraphs = [p.text.strip() for p in driver.find_elements(By.TAG_NAME, "p") if p.text.strip()]

    sentences = []
    for para in paragraphs:
        for sent in nltk.sent_tokenize(para):
            if len(sent.split()) > 3:  # only keep sentences with >3 words
                sentences.append(sent.strip())

    with open("nytimes_paragraphs_selenium.txt", "w", encoding="utf-8") as f:
        for idx, sent in enumerate(sentences, start=1000):  # start DocID at 1000
            f.write(f"{idx}\t{sent}\n")

    print(f"✅ Extracted {len(sentences)} sentences and saved to file")
finally:
    driver.quit()
