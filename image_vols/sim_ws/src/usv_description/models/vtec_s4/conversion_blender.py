import bpy
import xml.etree.ElementTree as ET
import mathutils
import os

# 1. Define your absolute base directory
base_dir = "/home/mwlgt/vanttec_usv_gz/image_vols/sim_ws/src/usv_description/models/vtec_s4"
sdf_path = os.path.join(base_dir, "model.sdf")

# Parse the SDF XML
tree = ET.parse(sdf_path)
root = tree.getroot()

# Deselect all existing objects in Blender to avoid accidental transformations
bpy.ops.object.select_all(action='DESELECT')

# Find all  tags
for visual in root.iter('visual'):
    uri_elem = visual.find('.//uri')
    if uri_elem is None: 
        continue
    
    # 2. Convert the SDF URI to your actual Linux file path
    # Example: 'file://vtec_s4/meshes/asv.dae' -> '/home/mwlgt/.../meshes/asv.dae'
    raw_uri = uri_elem.text
    dae_path = raw_uri.replace('file://vtec_s4', base_dir)
    
    # Check if we are dealing with a .dae file and if it actually exists
    if not dae_path.endswith('.dae'):
        print(f"Skipping non-DAE file: {dae_path}")
        continue
        
    if not os.path.exists(dae_path):
        print(f"Error: File not found at {dae_path}")
        continue

    # 3. Import the DAE mesh
    bpy.ops.wm.collada_import(filepath=dae_path)
    
    # Blender automatically selects newly imported objects
    imported_objs = bpy.context.selected_objects
    
    # 4. Apply the  coordinates (X Y Z Roll Pitch Yaw)
    pose_elem = visual.find('.//pose')
    if pose_elem is not None:
        pose_vals = [float(v) for v in pose_elem.text.split()]
        loc = mathutils.Vector((pose_vals[0], pose_vals[1], pose_vals[2]))
        euler = mathutils.Euler((pose_vals[3], pose_vals[4], pose_vals[5]), 'XYZ')
        
        for obj in imported_objs:
            obj.location = loc
            obj.rotation_euler = euler
            
    # Deselect the current mesh so the next loop starts clean
    bpy.ops.object.select_all(action='DESELECT')

print("SDF visuals imported successfully!")