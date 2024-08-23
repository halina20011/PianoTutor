import os
import bpy, bmesh
import json
import math
import struct

folder = bpy.path.abspath("//");
print(folder);
jsonFile = os.path.join(folder, "glyphs.json");
imagesFolder = os.path.join(folder, "Images");
print(jsonFile);

font = bpy.data.fonts["Emmentaler-11 11"];
# font = bpy.data.fonts["Emmentaler-Brace Brace"];

def readJson(file):
    with open(file, "r") as f:
        return json.load(f);

folder = bpy.path.abspath("//");
assetsJson = os.path.join(folder, "assets.json");
models = readJson(assetsJson);
modelsFile = os.path.join(folder, "models.bin");

class Vertex:
    def __init__(self, co):
        self.x = co.x;
        self.z = co.z;
        self.y = co.y;

    def data(self):
        return [self.x, self.y, self.z];

def exportAsImg(name):
    path = os.path.join(imagesFolder, name);
    print(f"image {path}");
    bpy.context.scene.render.filepath = path
    bpy.ops.render.render(write_still=True)

def loadFont(charInt):
    bpy.ops.object.text_add(location=(0,0,0));
    textObj = bpy.context.object;
    textObj.data.body = chr(charInt);
    textObj.data.font = font;

def takePhoto(char, key):
    bpy.ops.object.text_add(location=(0,0,0));
    textObj = bpy.context.object;
    textObj.data.body = char;
    textObj.data.font = font;
    bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='MEDIAN');
    bpy.context.object.location[0] = 0;
    bpy.context.object.location[1] = 0;
    bpy.context.object.location[2] = 0;
    exportAsImg(key);
    bpy.ops.object.delete(use_global=False, confirm=False)

def all():
    with open(jsonFile, "r") as f:
        dict = json.load(f);

        print(dict);
        size = len(dict)
        print(size);
        row = math.ceil(math.sqrt(size));
        str = "";
        for i, key in enumerate(dict):
            print(f"exporting {i} from {size}");
            char = chr(dict[key]);
            
            takePhoto(char, key);
            # str += char;
            # if(i != 0 and i % row == 0):
            #     str += '\n';

class Item:
    def __init__(self, modelName):
        self.name = modelName;
        self.modelId = modelName;
        self.obj = bpy.context.scene.objects.get(modelName);
        if(self.obj == None):
            raise ValueError(f"{modelName} does not exist");
        
        self.getData();

    def export(self, file):
        # https://docs.python.org/3/library/struct.html#struct-format-strings
        file.write(struct.pack("B", len(self.modelId)));
        file.write(struct.pack(f"{len(self.modelId)}s", self.modelId.encode()));
        file.write(struct.pack("i", len(self.data)));
        file.write(struct.pack(f"{len(self.data)}f", *self.data));
        print(f"exported {self.name} {len(self.data)}");

    def getData(self):
        # Check if the object is a mesh
        if(self.obj.type != "MESH"):
            raise ValueError(f"{self.name} is not a mesh");
            return;

        mesh = self.obj.data;

        graph = bpy.context.evaluated_depsgraph_get();
        evalObj = mesh.evaluated_get(graph);
        meshData = self.obj.data;
        
        bm = bmesh.new();
        bm.from_mesh(meshData);
        
        bmesh.ops.triangulate(bm, faces=bm.faces[:]);
        
        if not mesh.uv_layers:
            return
        
        uv_lay = bm.loops.layers.uv.active;

        # uv_layer = mesh.uv_layers.active.data;
        data = [];
        for face in bm.faces:
            for loop in face.loops:
                vert = loop.vert;
                for v in Vertex(vert.co).data():
                    data.append(v);

        del(evalObj);
        bm.free();
        
        self.data = data;

items = [];

def printSelected():
    for obj in bpy.context.selected_objects:
        item = Item(obj.name);
        print(item.data);

def getObjects():
    models["text"] = {};
    for i in range(10):
        models["text"][str(i)] = str(i);
    
    models["notes"] = {};
    for i in range(12):
        noteName = f"note{chr(ord('A') + i)}";
        models["notes"][noteName] = noteName;

    for itemType in models:
        types = models[itemType];
        for modelName in types:
            item = Item(modelName);
            items.append(item);

        print(item.name);

def exportAll(items):
    with open(modelsFile, "wb") as f:
        f.write(struct.pack("i", len(items)));
        for item in items:
            item.export(f);

# printSelected();
getObjects();
exportAll(items);

# loadFont(57623);
