# Milestone 2
# David Kol 49938855
# Justin Ho
import math
import json
import sys
import os
import lxml.html
import lxml.html.soupparser
import nltk
from nltk.tokenize import RegexpTokenizer

def ExtractHTMLContent(HTML):
    try:
        getHTMLroot(HTML).text_content().encode('ascii', errors='ignore')
    except Exception:
        return ''
    return getHTMLroot(HTML).text_content().encode('ascii', errors='ignore')

def ExtractHTMLTitle(HTML):
    root = getHTMLroot(HTML)
    title = ''
    for element in root.iter('title'):
        title = element.text
    if title == None:
        title = ''
    title = title.lstrip().rstrip()
    return title

def getHTMLroot(HTML):
    if HTML == '':
        return ''
    try:
        root = lxml.html.fromstring(HTML)
    except Exception:
        return ''
    try:
        ignore = lxml.html.tostring(root, encoding='unicode')
    except UnicodeDecodeError:
        root = lxml.html.soupparser.fromstring(HTML)
    return root

def FileToHTML(path):
    return open(path, 'r').read()

def getTokens(string):
    return tokenizer.tokenize(string)    

def generateDocFreq(path):
    global file_count
    global documentLengths
    global df
    if os.path.isfile(path):
        file_count += 1
        content = ExtractHTMLContent(FileToHTML(path))
        tokens = getTokens(content)
        unique_set = set()
        for ele in tokens:
            if ele not in unique_set:
                unique_set.add(ele)
                if df.has_key(ele):
                    df[ele] += 1
                else:
                    df[ele] = 1

        documentLengths[stripPath(path)] = len(unique_set)

    elif os.path.isdir(path):
        for new_path in os.listdir(path):
            generateDocFreq(os.path.join(path, new_path))

    

def generateTermFreq(tokens):
    tf = {}

    for ele in tokens:
        if not tf.has_key(ele):
            tf[ele] = 1
        else:
            tf[ele] += 1

    for key, value in tf.items():
        tf[key] = 1 + math.log10(value) #log scale of term frequency
    
    return tf

def stripPath(path):
    newpath = path.split('/')
    return (newpath[-1].replace("\\", "/"))

def createIndex(path):
    global index_count
    global df
    global index
    if os.path.isfile(path):
        index_count += 1
        content = ExtractHTMLContent(FileToHTML(path))
        tokens = getTokens(content)
        tf = generateTermFreq(tokens)
        sp = stripPath(path)
        for key, value in tf.items():
            if not index.has_key(key):
                index[key] = {sp : (value * df[key])}
            else:
                index[key][sp] = (value * df[key])


    elif os.path.isdir(path):
        for new_path in os.listdir(path):
            createIndex(os.path.join(path, new_path))


def generateIndex():
    global df
    print "No index found, creating index"

    generateDocFreq(DIRECTORY)
    for term, freq in df.items():
        df[term] = math.log10(file_count/freq)
    createIndex(DIRECTORY)

    print 'Writing to index.json'
    with open('index.json', 'w') as f:
        for chunk in json.JSONEncoder().iterencode(index):
            f.write(chunk)
	

    print 'Writing to documentLengths.json'
    with open('documentLengths.json', 'w') as f:
        for chunk in json.JSONEncoder().iterencode(documentLengths):
            f.write(chunk)

def getLinks(query):
    global index
    rankedfindings = {}

    for word in query.split(' '):
        if not index.has_key(word):
            print "word not found"
            continue
        for doc, idftf in index[word].items():
            if not rankedfindings.has_key(doc):
                rankedfindings[doc] = idftf
            else:
                rankedfindings[doc] += idftf

    for doc, idftf in rankedfindings.items(): #normalize idftf
        rankedfindings[doc] = idftf / documentLengths[doc]

    sorted_rankedfindings = sorted(rankedfindings.items(), key=lambda x: -x[1])
    count = len(sorted_rankedfindings)
    if count > 10:
        count = 10
    for i in range(count):
        HTML = FileToHTML(DIRECTORY + sorted_rankedfindings[i][0])
        content = ExtractHTMLContent(HTML)
        title = ExtractHTMLTitle(HTML)
        tokens = getTokens(content)
        print
        print
        print i+1, ':', title 
        print "URL :", bookkeep[sorted_rankedfindings[i][0]]
        print "Rank:", sorted_rankedfindings[i][1]
        print "first 10 tokens:", tokens[0:10]
    print
 
DIRECTORY = "./WEBPAGES_RAW/"
file_count = 0
index_count = 0
documentLengths = {}
df = {}
index = {}
tokenizer = RegexpTokenizer(r'[a-zA-Z0-9]+')
if not os.path.exists("./index.json"):
    generateIndex()
    print "index complete, run again to query"
else:
    print "preparing query system"
    index = json.loads(open("./index.json").read())
    documentLengths = json.loads(open("./documentLengths.json").read())
    bookkeep = json.loads(open("./bookkeeping.json").read())
    while 1:
        query = raw_input("Please type a search query: ").lower()
        getLinks(query)

