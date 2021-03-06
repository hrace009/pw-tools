#pragma once

#include "bon.h"
#include "options.h"
#include "ecmHead.h"
#include "ecmAct.h"
#include "ecmScript.h"
#include "ecmSkin.h"
#include "ecmChild.h"

using namespace System;
using namespace System::IO;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Text;

public ref class ECM
{
	public: ECM(void)
	{
	}

	protected: ~ECM()
	{
	}

	public: ecmHEAD^ header;
	public: ArrayList^ coGFXs;
	public: array<ecmACT^>^ combineActs;
	public: array<ecmSCRIPT^>^ scripts;
	public: array<ecmSKIN^>^ skins;
	public: array<ecmCHILD^>^ children;
	public: String^ mphys;
	public: String^ hooks;

	public: void Load(String^ file)
	{
		array<String^>^ linebreak = gcnew array<String^>{"\r\n"};
		array<String^>^ temp;
		StreamReader^ sr = gcnew StreamReader(file, Encoding::GetEncoding("GBK"));

		array<String^>^ blocks = sr->ReadToEnd()->Split(gcnew array<String^>{"CoGfxNum: ", "ScriptCount: ", "AddiSkinCount: ", "ChildCount: ", "PhysFileName: ", "ECMHookCount: "}, StringSplitOptions::RemoveEmptyEntries);
		sr->Close();

		// scripts are not suported in low moxtversions
		if(blocks->Length > 4)
		{
			blocks[1] = "CoGfxNum: " + blocks[1];
			blocks[2] = "ScriptCount: " + blocks[2];
			blocks[3] = "AddiSkinCount: " + blocks[3];
			blocks[4] = "ChildCount: " + blocks[4];
		}
		else
		{
			Array::Resize(blocks, blocks->Length+1);
			blocks[4] = "ChildCount: " + blocks[3];
			blocks[3] = "AddiSkinCount: " + blocks[2];
			blocks[2] = "ScriptCount: 0";
			blocks[1] = "CoGfxNum: " + blocks[1];			
		}

		// don't remove empty entries, because header may have one version <= 32
		header = gcnew ecmHEAD();
		header->SetAllParameters(blocks[0]->Split(linebreak, StringSplitOptions::None));

		temp = blocks[1]->Split(gcnew array<String^>{"CombineActName: "}, StringSplitOptions::RemoveEmptyEntries);

		// temp[1...n] contains the ComAct
		combineActs = gcnew array<ecmACT^>(temp->Length-1);
		for(int i=0; i<combineActs->Length; i++)
		{
			combineActs[i] = gcnew ecmACT();
			temp[i+1] = "CombineActName: " + temp[i+1];
			combineActs[i]->SetAllParameters((temp[i+1])->Split(linebreak, StringSplitOptions::RemoveEmptyEntries));
		}

		// temp[0] contains the CoGfx
		temp = temp[0]->Split(gcnew array<String^>{"EventType: "}, StringSplitOptions::RemoveEmptyEntries);
		coGFXs = gcnew ArrayList();
		for(int i=0; i<temp->Length-1; i++)
		{
			coGFXs->Add(gcnew ecmTYPE());
			temp[i+1] = "EventType: " + temp[i+1];
			((ecmTYPE^)coGFXs[i])->SetAllParameters((temp[i+1])->Split(linebreak, StringSplitOptions::RemoveEmptyEntries));
		}

		// temp[1...n] contains the Scripts
		temp = blocks[2]->Split(gcnew array<String^>{"id: "}, StringSplitOptions::RemoveEmptyEntries);
		scripts = gcnew array<ecmSCRIPT^>(temp->Length-1);
		for(int i=0; i<scripts->Length; i++)
		{
			scripts[i] = gcnew ecmSCRIPT();
			temp[i+1] = "id: " + temp[i+1];
			scripts[i]->SetAllParameters((temp[i+1])->Split(linebreak, StringSplitOptions::RemoveEmptyEntries));
		}

		// temp[1...n] contains the Skins
		temp = blocks[3]->Split(gcnew array<String^>{"AddiSkinPath: "}, StringSplitOptions::RemoveEmptyEntries);
		skins = gcnew array<ecmSKIN^>(temp->Length-1);
		for(int i=0; i<skins->Length; i++)
		{
			skins[i] = gcnew ecmSKIN();
			temp[i+1] = "AddiSkinPath: " + temp[i+1];
			skins[i]->SetAllParameters((temp[i+1])->Split(linebreak, StringSplitOptions::RemoveEmptyEntries));
		}

		// temp[1...n] contains the Children
		temp = blocks[4]->Split(gcnew array<String^>{"ChildName: "}, StringSplitOptions::RemoveEmptyEntries);
		children = gcnew array<ecmCHILD^>(temp->Length-1);
		for(int i=0; i<children->Length; i++)
		{
			children[i] = gcnew ecmCHILD();
			temp[i+1] = "ChildName: " + temp[i+1];
			children[i]->SetAllParameters((temp[i+1])->Split(linebreak, StringSplitOptions::RemoveEmptyEntries));
		}

		// block[5] contains the mphys (invalid 1.3.6 block)
		if(blocks->Length > 5)
		{
			mphys = "PhysFileName: " + blocks[5];
		}

		// block[6] contains the hooks (invalid 1.3.6 blocks)
		if(blocks->Length > 6)
		{
			hooks = "ECMHookCount: " + blocks[6];
		}
	}

	public: void Fix(Options^ options)
	{
		header->Fix(options);
		for(int i=0; i<coGFXs->Count; i++)
		{
			// 104 don't appear in cn 1.3.6, also there are no ATT files
			// a bugged EventType: 104 was added as MOXTVersion: 32 in PWI
			// the att files for this file was missing
			// i assume this EventType: 104 was corrupted, so we will remove them...
			if(((ecmTYPE^)coGFXs[i])->Version > 103)
			{
				coGFXs->RemoveAt(i);
				i--;
			}
			else
			{
				((ecmTYPE^)coGFXs[i])->Fix(options);
			}
		}
		for(int i=0; i<combineActs->Length; i++)
		{
			combineActs[i]->Fix(options);
		}
		for(int i=0; i<scripts->Length; i++)
		{
			scripts[i]->Fix(options);
		}
		for(int i=0; i<skins->Length; i++)
		{
			skins[i]->Fix(options);
		}
		for(int i=0; i<children->Length; i++)
		{
			children[i]->Fix(options);
		}
		if(options->ecmSoulsphereHook && combineActs->Length > 0 && options->fileIn->Contains(Encoding::GetEncoding("GBK")->GetString(gcnew array<unsigned char>{200, 203,206, 239,92, 216, 176, 202, 215, 92, 183, 168, 177, 166, 92})))
		{
			// grab eventtypes from the default combineact and add them to the coGFX
			// assumption: the first combineact is always the default
			array<String^>^ temp = combineActs[0]->GetAllParameters();
			ArrayList^ types = gcnew ArrayList();

			for(int i=0; i<temp->Length; i++)
			{
				if(temp[i]->StartsWith("EventType:"))
				{
					types->Add(gcnew ArrayList());
				}
				if(types->Count > 0)
				{
					((ArrayList^)types[types->Count-1])->Add(temp[i]);
				}
			}

			for(int i=0; i<types->Count; i++)
			{
				ecmTYPE^ type = gcnew ecmTYPE();
				type->SetAllParameters(reinterpret_cast<array<String^>^>(((ArrayList^)types[i])->ToArray(String::typeid)));
				coGFXs->Add(type);
			}

			if(File::Exists(options->fileIn->Replace(".ecm", ".bon")))
			{
				Encoding^ enc = Encoding::UTF8;
				BON^ b = gcnew BON();
				b->Load(options->fileIn->Replace(".ecm", ".bon"));
				for(int i=0; i<b->Hooks->Length; i++)
				{
					if(enc->GetString(b->Hooks[i]->Name) == "HH_shang")
					{
						b->Hooks[i]->Name = enc->GetBytes("CC_weapon");
						b->Hooks[i]->TransformationMatrix[12] += Convert::ToSingle(options->ecmSoulsphereHookTX);
						b->Hooks[i]->TransformationMatrix[13] += Convert::ToSingle(options->ecmSoulsphereHookTY);
						b->Hooks[i]->TransformationMatrix[14] += Convert::ToSingle(options->ecmSoulsphereHookTZ);
					}
				}
				b->Save(options->fileOut->Replace(".ecm", ".bon"));
			}
		}
	}

	public: void Save(String^ file)
	{
		array<String^>^ temp;

		StreamWriter^ sw = gcnew StreamWriter(file, false, Encoding::GetEncoding("GBK"));

		temp = header->GetAllParameters();

		// save the header to file
		for(int i=0; i<temp->Length; i++)
		{
			sw->WriteLine(temp[i]);
		}

		// save cogfx and combineacts
		sw->WriteLine("CoGfxNum: " + coGFXs->Count);
		sw->WriteLine("ComActCount: " + combineActs->Length);
		for(int n=0; n<coGFXs->Count; n++)
		{
			temp = ((ecmTYPE^)coGFXs[n])->GetAllParameters();
			for(int i=0; i<temp->Length; i++)
			{
				sw->WriteLine(temp[i]);
			}
		}
		for(int n=0; n<combineActs->Length; n++)
		{
			temp = combineActs[n]->GetAllParameters();
			for(int i=0; i<temp->Length; i++)
			{
				sw->WriteLine(temp[i]);
			}
		}

		// save scripts
		sw->WriteLine("ScriptCount: " + scripts->Length);
		for(int n=0; n<scripts->Length; n++)
		{
			temp = scripts[n]->GetAllParameters();
			for(int i=0; i<temp->Length; i++)
			{
				sw->WriteLine(temp[i]);
			}
		}

		// save skins
		sw->WriteLine("AddiSkinCount: " + skins->Length);
		for(int n=0; n<skins->Length; n++)
		{
			temp = skins[n]->GetAllParameters();
			for(int i=0; i<temp->Length; i++)
			{
				sw->WriteLine(temp[i]);
			}
		}

		// save children
		sw->WriteLine("ChildCount: " + children->Length);
		for(int n=0; n<children->Length; n++)
		{
			temp = children[n]->GetAllParameters();
			for(int i=0; i<temp->Length; i++)
			{
				sw->WriteLine(temp[i]);
			}
		}

		sw->Close();
	}
};