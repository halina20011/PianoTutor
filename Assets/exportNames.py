#!/bin/python

import xml.etree.ElementTree as ET
import os
import json

def getLeafs(element):
    leafs = []
    if len(element) == 0:
        leafs.append(element);
    else:
        for child in element:
            leafs.extend(getLeafs(child));
    return leafs

directory = './Fonts/';

xmlFiles = [os.path.join(directory, file) for file in os.listdir(directory) if file.endswith('.xml')];

# def listLeafs():
#     dict = {};
#     for xmlFile in xmlFiles:
#         tree = ET.parse(xmlFile);
#         root = tree.getroot();
#         elements = get_leaf_elements(root);
#
# listLeafs();

def listGlyphs(file):
    tree = ET.parse(file);
    root = tree.getroot();
    elements = getLeafs(root);
    print(len(elements));
    dict = {};
    nameToCode = {};
    for e in elements:
        attrib = e.attrib;
        if("name" in attrib and "code" in attrib):
            name = e.attrib["name"];
            code = int(e.attrib["code"], 16);
            if(name not in nameToCode):
                nameToCode[name] = code;

            if(name not in dict):
                dict[name] = 0;
            
            dict[name] += 1;

    for key in dict:
        print(key, dict[key]);

    exportJson(nameToCode);

def exportJson(dict):
    with open("glyphs.json", "w") as f:
        json.dump(dict, f);

listGlyphs("./Fonts/emmentaler-11.otf.xml");
# listGlyphs("./Fonts/emmentaler-brace.otf.xml");
