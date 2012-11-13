//--------------------------------------------------------------------------
// $Id$
//--------------------------------------------------------------------------
//
// File:        PyEntity.C
// Module:      Python
// Author:      Sunil Thulasidasan
// Created:     July 31 2012 
// Description: Base class for entities in Python
//
// @@COPYRIGHT@@
//
//--------------------------------------------------------------------------

#include <boost/python.hpp>
//#include <boost/make_shared.hpp>

#include "simx/Python/PyEntity.h"
#include "simx/Python/PyUtility.h"
#include "simx/Python/PyInfo.h"
#include "simx/InfoManager.h"
#include "simx/Python/PyService.h"


using namespace std;
using namespace boost;

namespace simx{
  namespace Python {
    
    //(TODO: python high). need to get rid of this hack)
    python::object none_object = python::api::object();

    // a wrapper around the helper function distributeInfo
    int distributePyInfo( const PyEntity& ent, const python::object& py_info,
			  const PyService& serv, bool exclude )
    {
      shared_ptr<PyInfo> info;
      theInfoManager().createInfo( info );
      //info->fData = &py_info;
      info->setData( py_info );
      //info->fData =  boost::make_shared<boost::python::object>(py_info);
      if ( exclude )
	return distributeInfo( ent, info, &serv );
      else
	return distributeInfo( ent, info );
    }


    
    PyEntity::PyEntity( const EntityID& id, 
			LP& lp, const PyEntityInput& input,
			const python::object& obj)
      : Entity(id, lp, input), fPyObj(obj), fPyId( EntityId2py( getId() ))
    {
      
      //      cout << "c++ pyentity " << id << " my address is " << long(this) << endl;
      Logger::debug3() << "I'm a c++ PyEntity" << std::endl;
      i = id.get<1>();
      //createServices(*this,input.fServices);
      i = i+1;
      // boost::shared_ptr<PyService> py_serv;
      // if (! getService( static_cast<ServiceAddress>(11101), py_serv ) )
      // 	assert(false);
    }

    void PyEntity::print_address() const
    {
      //cout << "c++ pyentity " << getId() << " my address is " << long(this) << endl;
    }
    
    
    python::object& PyEntity::getPyService(long serv_addr)
    {
      shared_ptr<PyService> py_serv;
      if (getService( static_cast<ServiceAddress>(serv_addr), py_serv ) ) 
	{

	  //return python::api::object(*py_serv);
	  //return none_object;
	  return py_serv->getPyObject();
	}
      else
	//return new python::api::object(); //returns None object to python
	//TODO: return api::object()
	return none_object;
	
		     
    }

    python::tuple PyEntity::getPyId() const
    {
      return fPyId;
    }

    void PyEntity::sendPyInfo(const python::object& py_info,
					 const Time time,
					 const python::tuple& dest_ent,
					 const long dest_serv)
    {
      EntityID e_id = py2EntityId( dest_ent );
      shared_ptr<PyInfo> info;
      theInfoManager().createInfo( info );
      //info->fData = boost::make_shared<boost::python::object>(py_info);// py_info;
      //info->fData = &py_info;
      info->setData( py_info );
      processOutgoingInfo( info, time, e_id, 
			   static_cast<ServiceAddress>( dest_serv ) );
    }


    python::object& PyEntity::getPyObj() {
      return fPyObj;
    }

    void PyEntity::createPyServices( const PyEntityInput& input )
    {
      createServices(*this,input.fServices);
    }

    // function to execute call-backs in python
    void PyEntity::pyCallBack( tuple<string,
			       python::tuple>& cb_tuple ) {

      // string is the attribute of fPyObj that will be called.
      // There is no guarantee that fPyObj will have such an attribute,
      // so we could test using hasattr, or we could just go ahead and call
      // (assuming that it will likely have such a method) and catch 
      // the exception, since in Python "..'tis easier to ask for 
      // forgiveness than permission" (EAFP)
      const char* atr = cb_tuple.get<0>().c_str();
      try {
	fPyObj.attr( atr )( cb_tuple.get<1>() );
      }
      
      catch (const python::error_already_set& e) {
	
	Logger::error() << "PyEntity: pyCallBack(): Python error occured  "
			<< "while trying to call method " << atr
			<< " for entity " << getId() << endl;

	PyErr_Print();
	PyErr_Clear();
      }
		     
    }

  } //namespace simx
} //namespace Python


using namespace boost::python;

void export_PyEntity() {


  python::class_<simx::EntityID>("EntityID",python::no_init);
  
  python::class_<simx::Python::PyEntity,
    python::bases<simx::Entity> >
    ("PyEntity",python::init
				     <const simx::EntityID&,
				     simx::LP&,
				     const simx::Python::
				      PyEntityInput&,
				      const python::object&>() )
    .def("getId",&simx::Python::PyEntity::getPyId)
    .def("send_info",&simx::Python::PyEntity::sendPyInfo)
    .def("get_service",&simx::Python::PyEntity::getPyService,
	 return_value_policy<copy_non_const_reference>())
    .def("create_services",&simx::Python::PyEntity::createPyServices)
    .def("print_address",&simx::Python::PyEntity::print_address)
    .def_readwrite("i_",&simx::Python::PyEntity::i)
    
    ;
  def("distribute_info", &simx::Python::distributePyInfo );

}