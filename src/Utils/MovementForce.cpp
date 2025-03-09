#include "Utils/MovementForce.hpp"

using namespace GTS;

// This file is outdated and doesn't work as expected, so all functions just return 1.0f for now 
// Needs rework

namespace {
	/*
	NodeMovementType Convert_To_MovementType(DamageSource Source) {

		NodeMovementType Type = NodeMovementType::Movement_None;

		switch (Source) {

			case DamageSource::HandDropRight:
			case DamageSource::HandSwipeRight:
			case DamageSource::HandSlamRight:
			case DamageSource::HandCrawlRight:
				Type = NodeMovementType::Movement_RightHand;
			break;	
			case DamageSource::HandDropLeft:
			case DamageSource::HandSwipeLeft:
			case DamageSource::HandSlamLeft:
			case DamageSource::HandCrawlLeft:
				Type = NodeMovementType::Movement_LeftHand;
			break;
			case DamageSource::WalkRight:
			case DamageSource::FootIdleR:
			case DamageSource::CrushedRight:
			case DamageSource::FootGrindedRight:
			case DamageSource::KickedRight:
			case DamageSource::KneeRight:
				Type = NodeMovementType::Movement_RightLeg;
			break;	
			case DamageSource::WalkLeft:
			case DamageSource::FootIdleL:
			case DamageSource::CrushedLeft:
			case DamageSource::FootGrindedLeft:
			case DamageSource::KickedLeft:
			case DamageSource::KneeLeft:
				Type = NodeMovementType::Movement_LeftLeg;
			break;	
			default: {
				return NodeMovementType::Movement_None;
			break;
			}
		}
		return Type;
	}

	float Record_Node_Coordinates(NiAVObject* Node, NiPoint3& coords_out) {
		float NodeMovementForce = 0.0f;
		if (Node) {
			NiPoint3 coords_in = Node->world.translate;

			//log::info("Input coords: {}", Vector2Str(coords_in));
			if (coords_in.Length() > 0 && coords_out.Length() > 0) {
				NodeMovementForce = (coords_in - coords_out).Length();
				// ^ Compare values, get movement force of Node X over 1 frame
			}
			//log::info("Output coords: {}", Vector2Str(coords_out));
			if (coords_in == coords_out) { // We don't want to apply it on the same frame in that case, will result in 0
				return NodeMovementForce;
			} else {
				coords_out = coords_in;
			}
			// Else Record new pos of bone
		}
		
		return NodeMovementForce;
	}
	*/
}

namespace GTS {

	float Get_Bone_Movement_Speed(Actor* actor, NodeMovementType type) {

		return 1.0f;

		/*
		auto profiler = Profilers::Profile("MovementForce: GetBoneMovementSpeed");
		NiAVObject* Node = nullptr;

		float NodeMovementForce = 0.0f;
		float scale = get_visual_scale(actor);
		
		auto Data = Transient::GetSingleton().GetData(actor);

		if (Data) {

			NiPoint3& DataCoordinates_LL = Data->POSLastLegL;
			NiPoint3& DataCoordinates_RL = Data->POSLastLegR;
			NiPoint3& DataCoordinates_LH = Data->POSLastHandL;
			NiPoint3& DataCoordinates_RH = Data->POSLastHandR;

			switch (type) {
				case NodeMovementType::Movement_LeftLeg: {
					//log::info("-------for Left Leg: ");
					Node = find_node(actor, "NPC L Foot [Lft ]");
					NodeMovementForce = Record_Node_Coordinates(Node, DataCoordinates_LL);
					break;
				}
				case NodeMovementType::Movement_RightLeg: {
					//log::info("-------for Right Leg: ");
					Node = find_node(actor, "NPC R Foot [Rft ]");
					NodeMovementForce = Record_Node_Coordinates(Node, DataCoordinates_RL);
					break;
				}
				case NodeMovementType::Movement_LeftHand: 
					Node = find_node(actor, "NPC L Hand [LHnd]");
					NodeMovementForce = Record_Node_Coordinates(Node, DataCoordinates_LH);
				break;
				case NodeMovementType::Movement_RightHand: 
					Node = find_node(actor, "NPC R Hand [RHnd]");
					NodeMovementForce = Record_Node_Coordinates(Node, DataCoordinates_RH);
				break;
				case NodeMovementType::Movement_None:
					return 1.0f; // Always allow for actions that are supposed to stagger always
				break;
			}
		}
		
		if (NodeMovementForce > 0) {
			//log::info("movement force: {}", NodeMovementForce);
			float NodeMovementForce_Clamped = std::clamp(NodeMovementForce / 10.0f, 0.0f, 1.0f);
			//log::info("Clamped movement force: {}", NodeMovementForce_Clamped);
			return NodeMovementForce_Clamped;
		}

		return 0.0f;
		*/
	}

	float Get_Bone_Movement_Speed(Actor* giant, DamageSource Source) {
		return 1.0f;

		/*
		auto profiler = Profilers::Profile("MovementForce: GetBoneMovementSpeed");
		NodeMovementType Type = Convert_To_MovementType(Source);

		//if (giant->formID == 0x14) {
		//	log::info("Returning type: {}", static_cast<int>(Type));
		//}


		return Get_Bone_Movement_Speed(giant, Type);
		*/
	}
}