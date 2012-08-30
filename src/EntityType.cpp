#include "BoostPythonSM.h"
#include "EntityType.h"
#include "Macros.h"
#include "CompatWrappers.h"
#include "ViperExtension.h"
#include "InvalidEntityExceptionType.h"

namespace py = boost::python;

EntityType::EntityType(int index, std::string entityTrackerGuid, CBaseEntity *entity) {
	if(entity == NULL) {
		throw InvalidEntityExceptionType();
	}

	Index = index;
	BaseEntity = entity;
	EntityTrackerGuid = entityTrackerGuid;
}

bool EntityType::IsValid() {
	if(BaseEntity == NULL) {
		return false;
	}

	edict_t *edict = g_Interfaces.ServerGameEntsInstance->BaseEntityToEdict(BaseEntity);

	if(edict == NULL || edict->IsFree()) {
		return false;
	}

	return true;
}