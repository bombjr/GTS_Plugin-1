#include "UI/DebugAPI.hpp"
#include "rays/raycast.hpp"
#include "rays/allcollector.hpp"

using namespace Gts;
using namespace RE;

namespace {
	void CastRayImpl(TESObjectREFR* ref, const NiPoint3& in_origin, const NiPoint3& direction, const float& unit_length, AllRayCollector* collector) {
		float length = unit_to_meter(unit_length);
		if (!ref) {
			return;
		}
		auto cell = ref->GetParentCell();
		if (!cell) {
			return;
		}
		auto collision_world = cell->GetbhkWorld();
		if (!collision_world) {
			return;
		}
		bhkPickData pick_data;

		NiPoint3 origin = unit_to_meter(in_origin);
		pick_data.rayInput.from = origin;

		NiPoint3 normed = direction / direction.Length();
		NiPoint3 end = origin + normed * length;
		pick_data.rayInput.to = end;

		NiPoint3 delta = end - origin;
		pick_data.ray = delta; // Length in each axis to travel

		pick_data.rayInput.enableShapeCollectionFilter = false; // Don't bother testing child shapes
		pick_data.rayInput.filterInfo = collector->filterInfo;

		pick_data.rayHitCollectorA8 = collector;

		collision_world->PickObject(pick_data);

		for (auto& ray_result: collector->GetHits()) {
			ray_result.position = meter_to_unit(origin + normed * length * ray_result.hitFraction);
		}
		std::ranges::sort(collector->GetHits(), [](const AllRayCollectorOutput &a, const AllRayCollectorOutput &b)
		{
			return a.hitFraction < b.hitFraction;
		});
	}
}

namespace Gts {

	RayResult CastCamRay(glm::vec4 start, glm::vec4 end, float traceHullSize)
		noexcept {
		RayResult res;

		const auto ply = RE::PlayerCharacter::GetSingleton();
		const auto cam = RE::PlayerCamera::GetSingleton();
		if (!ply->parentCell || !cam->unk120) return res;

		auto physicsWorld = ply->parentCell->GetbhkWorld();
		if (physicsWorld) {
			typedef bool(__fastcall* RayCastFunType)(
				decltype(RE::PlayerCamera::unk120) physics, RE::bhkWorld* world, glm::vec4& rayStart,
				glm::vec4& rayEnd, uint32_t* rayResultInfo, RE::Character** hitCharacter, float traceHullSize
				);
			static auto cameraCaster = REL::Relocation<RayCastFunType>(RELOCATION_ID(32270, 33007));
			res.hit = cameraCaster(
				cam->unk120, physicsWorld,
				start, end, static_cast<uint32_t*>(res.data), &res.hitCharacter,
				traceHullSize
			);
		}

		if (res.hit) {
			res.hitPos = end;
			res.rayLength = glm::length(static_cast<glm::vec3>(res.hitPos) - static_cast<glm::vec3>(start));
		}

		return res;
	}

	// Performs a ray cast and returns a new position based on the result
	NiPoint3 ComputeRaycast(const NiPoint3& rayStart, const NiPoint3& rayEnd, const float hullMult) {
		const auto rayStart4 = glm::vec4(rayStart.x, rayStart.y, rayStart.z, 0.0f);
		const auto rayEnd4 = glm::vec4(rayEnd.x, rayEnd.y, rayEnd.z, 0.0f);
		const auto result = CastCamRay(rayStart4, rayEnd4, camhullSize * hullMult);


		if (result.hit) {
			NiPoint3 ResHit = { result.hitPos.x, result.hitPos.y, result.hitPos.z };
			NiPoint3 ResNorm = { result.rayNormal.x, result.rayNormal.y, result.rayNormal.z };
			return ResHit + (ResNorm * glm::min(result.rayLength, camhullSize * hullMult));
		}
		else {
			return rayEnd;
		}
	}

	NiPoint3 CastRay(TESObjectREFR* ref, const NiPoint3& origin, const NiPoint3& direction, const float& length, bool& success) {
		auto collector = AllRayCollector::Create();
		collector->Reset();
		collector->filterInfo = bhkCollisionFilter::GetSingleton()->GetNewSystemGroup() << 16 | std::to_underlying(COL_LAYER::kLOS);
		CastRayImpl(ref, origin, direction, length, collector.get());

		if (collector->HasHit()) {
			for (auto& hit: collector->GetHits()) {
				// This varient just returns the first result
				success = true;
				return hit.position;
			}
		}

		success = false;
		return NiPoint3();
	}

	NiPoint3 CastRayStatics(TESObjectREFR* ref, const NiPoint3& origin, const NiPoint3& direction, const float& length, bool& success) {
		auto collector = AllRayCollector::Create();
		collector->Reset();
		collector->filterInfo = bhkCollisionFilter::GetSingleton()->GetNewSystemGroup() << 16 | std::to_underlying(COL_LAYER::kLOS);
		CastRayImpl(ref, origin, direction, length, collector.get());

		if (collector->HasHit()) {
			for (auto& hit: collector->GetHits()) {
				// This varient filters out the char ones
				
				auto collision_layer = static_cast<COL_LAYER>(hit.rootCollidable->broadPhaseHandle.collisionFilterInfo & 0x7F);
				int layer_as_int = static_cast<int>(collision_layer);

				if (collision_layer != COL_LAYER::kCharController && collision_layer != COL_LAYER::kWeapon && 
					layer_as_int != 56 && layer_as_int != 8) {
					// 8 = kBiped
					// 56 = Supposedly weapon collisions
					/*if (ref->formID == 0x14) {
						log::info("------Hitting Layer: {}, as int: {}", collision_layer, layer_as_int); // Weapons hit "unknown" layer :/
					}*/
					success = true;
					return hit.position;
				}
			}
		}

		success = false;
		return NiPoint3();
	}
}
