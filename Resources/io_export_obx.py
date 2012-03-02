#!BPY
# 
# -----
# 
# Core S2 Software Solutions
# License Details for Developers and Users 
# 
# -----
# 
# Open Source License
# 
# Copyright (c)2010, Core S2 Software Solutions. All rights reserved. Redistribution of this product
# in source or binary forms, with or without modifications, is permitted for educational or academic
# use only, and must be for non-profit and non-commercial redistribution, provided the following
# conditions are met: 1. This entire license, as a whole, must be included with the distribution of
# either source and/or binary form. 2. Neither the name "Core S2 Software Solutions" nor the author
# names may be removed from any material in source and/or binary form. 3. The Core S2 Software
# Solutions EULA must be included with this license.
# 
# -----
# 
# End User License Agreement
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS, AUTHORS, AND CONTRIBUTORS "AS IS". IN NO EVENT
# SHALL THE COPYRIGHT HOLDER, OR ANY AUTHORS, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION).
# 
# -----
# 
# Pydoc: http://www.blender.org/documentation/blender_python_api_2_60_6/
# 

"""
This script is an exporter to OBJ extended (obx) file format for the Dwarfcraft game.

Usage:

Select the objects you wish to export and run this script from "File->Export" menu.
Objex (obx) is a special file format for a Core S2 project. A game model is defined as
a series of triangles which, for each vertex, includes the position, color, and UV texture.
This only exports the model file, and not the texture. Use a *.cfg to define the model-texture
pair relationship.

"""

bl_info = {
	"name": "Wavefront Extended Obj (.obx)...",
	"description": "Exports *.obx model files for Dwarfcraft",
	"author": "Jeremy Bridon",
	"version": (1, 2),
	"blender": (2, 53, 0),
	"api": 31236,
	"location": "File > Export",
	"warning": "", # used for warning icon and text in addons panel
	"wiki_url": "",
	"tracker_url": "",
	"category": "Import-Export"}

# Imports
import os, math, datetime, mathutils
import bpy
from bpy_extras.io_utils import (ExportHelper, path_reference_mode, axis_conversion)
from bpy.props import (BoolProperty, FloatProperty, StringProperty, EnumProperty)

### Core Functions ###

# Give a mesh face, we return an array of the corrected
# triangle vertices data, and in parallel the UV texture coordinates
def faceToTriangles(vface, tface):
	triangles = []
	textures = []
	if (len(vface.vertices) == 4):
		triangles.append(vface.vertices[0])
		triangles.append(vface.vertices[1])
		triangles.append(vface.vertices[2])
		
		triangles.append(vface.vertices[2])
		triangles.append(vface.vertices[3])
		triangles.append(vface.vertices[0])
		
		if tface != None:
			textures.append(tface.uv1)
			textures.append(tface.uv2)
			textures.append(tface.uv3)
			
			textures.append(tface.uv3)
			textures.append(tface.uv4)
			textures.append(tface.uv1)
		else:
			textures.append(0)
			textures.append(0)
			textures.append(0)
			
			textures.append(0)
			textures.append(0)
			textures.append(0)
	
	else:
		triangles.append(vface.vertices[0])
		triangles.append(vface.vertices[1])
		triangles.append(vface.vertices[2])
		
		if tface != None:
			textures.append(tface.uv1)
			textures.append(tface.uv2)
			textures.append(tface.uv3)
		else:
			textures.append(0)
			textures.append(0)
			textures.append(0)
	
	return (triangles, textures)

# User has chosen a file to write to
def WriteFileObx(filename):
	
	# Objects...
	
	# Add extension if needed
	if not filename.lower().endswith('.obx'):
		filename += '.obx'
	
	# Export just the active object
	object = bpy.context.scene.objects.active
	
	# Retrieve the mesh
	mesh = object.to_mesh(bpy.context.scene, True, "PREVIEW")
	
	# Orientation matrix
	matrix = bpy.context.selected_objects[0].matrix_world.copy()
	rotmatrix = mathutils.Matrix.Rotation(math.radians(90.0), 4, 'X')
	
	# Build all vertex coordinate data and texture UV data
	vertices = []
	textures = []
	for i in range(len(mesh.faces)):
		
		vface = mesh.faces[i] # Face geometry
		tface = None
		if len(mesh.uv_textures) > 0:
			tface = mesh.uv_textures[0].data[mesh.faces[i].index] # Texture data
		
		data = faceToTriangles(vface, tface)
		
		vertices.extend(data[0])
		textures.extend(data[1])
	
	# Open file
	file = open(filename, 'w')
	
	# For each face (just triangle sets)
	for i in range(len(vertices)):
		
		# Pull out vertex-specific data
		pos = matrix * mesh.vertices[vertices[i]].co * rotmatrix
		uv = textures[i]
		
		# Write this data
		file.write("%.6f %.6f %.6f %.4f %.4f\n" % (pos.x, pos.y, pos.z, uv[0], uv[1]))
	
	# Close file
	file.close();
	
	# Done
	return {'FINISHED'}

### Graphical User Interface ###

class ExportOBX(bpy.types.Operator, ExportHelper):
	'''Save an Extended OBJ File'''
	
	bl_idname = "export_scene.obx"
	bl_label = 'Export OBX'
	bl_options = {'PRESET'}
	
	filename_ext = ".obx"
	filepath = StringProperty(name="File Path", description="Filepath used for exporting the .obx file", maxlen=1024, subtype='FILE_PATH')
	
	# Executing save
	def execute(self, context):
		return WriteFileObx(self.filepath)
	
	# Invoking UI
	def invoke(self, context, event):
		if not self.filepath:
			self.filepath = bpy.path.ensure_ext(bpy.data.filepath, ".obx")
		wm = context.window_manager
		wm.fileselect_add(self)
		return {'RUNNING_MODAL'}
	 
### Registration & Setup ###

# Menu details for this addon
def menu_func_export(self, context):
	self.layout.operator(ExportOBX.bl_idname, text="Extended OBJ (.obx)")

# Register module
def register():
	bpy.utils.register_module(__name__)
	bpy.types.INFO_MT_file_export.append(menu_func_export)

# Release modile
def unregister():
	bpy.utils.unregister_module(__name__)
	bpy.types.INFO_MT_file_export.remove(menu_func_export)

# Main application entry point
if __name__ == "__main__":
	register()
