/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "brushmodule.h"

#include "qerplugin.h"

#include "brush/TexDef.h"
#include "ibrush.h"
#include "ifilter.h"
#include "brush/BrushNode.h"
#include "brushmanip.h"

#include "preferencesystem.h"
#include "stringio.h"

#include "map.h"
#include "qe3.h"
#include "mainframe.h"
#include "preferences.h"

LatchedBool g_useAlternativeTextureProjection(false, "Use alternative texture-projection");
bool g_showAlternativeTextureProjectionOption = false;

bool getTextureLockEnabled()
{
  return g_brush_texturelock_enabled;
}

void Face_importSnapPlanes(bool value)
{
  Face::m_quantise = /*value ? quantiseInteger : */quantiseFloating;
}
typedef FreeCaller1<bool, Face_importSnapPlanes> FaceImportSnapPlanesCaller;

void Face_exportSnapPlanes(const BoolImportCallback& importer)
{
  importer(false/*Face::m_quantise == quantiseInteger*/);
}
typedef FreeCaller1<const BoolImportCallback&, Face_exportSnapPlanes> FaceExportSnapPlanesCaller;

void Brush_constructPreferences(PreferencesPage& page)
{
  page.appendCheckBox(
    "", "Snap planes to integer grid",
    FaceImportSnapPlanesCaller(),
    FaceExportSnapPlanesCaller()
  );
  /*page.appendEntry(
    "Default texture scale",
    g_texdef_default_scale
  );*/
  if(g_showAlternativeTextureProjectionOption)
  {
    page.appendCheckBox(
      "", "Use alternative texture-projection",
      LatchedBoolImportCaller(g_useAlternativeTextureProjection),
      BoolExportCaller(g_useAlternativeTextureProjection.m_latched)
    );
  }
}
void Brush_constructPage(PreferenceGroup& group)
{
  PreferencesPage page(group.createPage("Brush", "Brush Settings"));
  Brush_constructPreferences(page);
}
void Brush_registerPreferencesPage()
{
  PreferencesDialog_addSettingsPage(FreeCaller1<PreferenceGroup&, Brush_constructPage>());
}


void Brush_Construct()
{
  Brush_registerCommands();
  Brush_registerPreferencesPage();

  BrushClipPlane::constructStatic();
  BrushInstance::constructStatic();
  Brush::constructStatic(/*type*/);

  Brush::m_maxWorldCoord = g_MaxWorldCoord;
  BrushInstance::m_counter = &g_brushCount;

  GlobalPreferenceSystem().registerPreference("TextureLock", BoolImportStringCaller(g_brush_texturelock_enabled), BoolExportStringCaller(g_brush_texturelock_enabled));
  GlobalPreferenceSystem().registerPreference("BrushSnapPlanes", makeBoolStringImportCallback(FaceImportSnapPlanesCaller()), makeBoolStringExportCallback(FaceExportSnapPlanesCaller()));
  //GlobalPreferenceSystem().registerPreference("TexdefDefaultScale", FloatImportStringCaller(g_texdef_default_scale), FloatExportStringCaller(g_texdef_default_scale));

  GridStatus_getTextureLockEnabled = getTextureLockEnabled;
  g_texture_lock_status_changed = FreeCaller<GridStatus_onTextureLockEnabledChanged>();
}

void Brush_Destroy()
{
  Brush::m_maxWorldCoord = 0;
  BrushInstance::m_counter = 0;

  Brush::destroyStatic();
  BrushInstance::destroyStatic();
  BrushClipPlane::destroyStatic();
}

void Brush_clipperColourChanged()
{
  BrushClipPlane::destroyStatic();
  BrushClipPlane::constructStatic();
}

void BrushFaceData_fromFace(const BrushFaceDataCallback& callback, Face& face)
{
  _QERFaceData faceData;
  faceData.m_p0 = face.getPlane().planePoints()[0];
  faceData.m_p1 = face.getPlane().planePoints()[1];
  faceData.m_p2 = face.getPlane().planePoints()[2];
  faceData.m_shader = face.GetShader();
  faceData.m_texdef = face.getTexdef().m_projection.m_texdef;
  faceData.contents = face.getShader().m_flags.m_contentFlags;
  faceData.flags = face.getShader().m_flags.m_surfaceFlags;
  faceData.value = face.getShader().m_flags.m_value;
  callback(faceData);
}
typedef ConstReferenceCaller1<BrushFaceDataCallback, Face&, BrushFaceData_fromFace> BrushFaceDataFromFaceCaller;
typedef Callback1<Face&> FaceCallback;

class Quake3BrushCreator : public BrushCreator
{
public:
  scene::Node& createBrush()
  {
    return (new BrushNode)->node();
  }
  bool useAlternativeTextureProjection() const
  {
    return g_useAlternativeTextureProjection.m_value;
  }
  void Brush_forEachFace(scene::Node& brush, const BrushFaceDataCallback& callback)
  {
    ::Brush_forEachFace(*Node_getBrush(brush), FaceCallback(BrushFaceDataFromFaceCaller(callback)));
  }
  bool Brush_addFace(scene::Node& brush, const _QERFaceData& faceData)
  {
    Node_getBrush(brush)->undoSave();
    return Node_getBrush(brush)->addPlane(faceData.m_p0, faceData.m_p1, faceData.m_p2, faceData.m_shader, TextureProjection(faceData.m_texdef, BrushPrimitTexDef(), Vector3(0, 0, 0), Vector3(0, 0, 0))) != 0;
  }
};

Quake3BrushCreator g_Quake3BrushCreator;

BrushCreator& GetBrushCreator()
{
  return g_Quake3BrushCreator;
}

#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"


class BrushDependencies :
  public GlobalRadiantModuleRef,
  public GlobalSceneGraphModuleRef,
  public GlobalShaderCacheModuleRef,
  public GlobalSelectionModuleRef,
  public GlobalOpenGLModuleRef,
  public GlobalUndoModuleRef,
  public GlobalFilterModuleRef
{
};

class BrushDoom3API : public TypeSystemRef
{
  BrushCreator* m_brushdoom3;
public:
  typedef BrushCreator Type;
  STRING_CONSTANT(Name, "doom3");

  BrushDoom3API()
  {
    Brush_Construct(/*eBrushTypeDoom3*/); // greebo: is always Doom 3

     m_brushdoom3 = &GetBrushCreator();
  }
  ~BrushDoom3API()
  {
    Brush_Destroy();
  }
  BrushCreator* getTable()
  {
    return m_brushdoom3;
  }
};

typedef SingletonModule<BrushDoom3API, BrushDependencies> BrushDoom3Module;
typedef Static<BrushDoom3Module> StaticBrushDoom3Module;
StaticRegisterModule staticRegisterBrushDoom3(StaticBrushDoom3Module::instance());
