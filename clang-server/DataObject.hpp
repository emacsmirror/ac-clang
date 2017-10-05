/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2017/10/05.18:29:16 */


#pragma once

#ifndef __DATA_OBJECT_HPP__
#define __DATA_OBJECT_HPP__



/*================================================================================================*/
/*  Comment                                                                                       */
/*================================================================================================*/


/*================================================================================================*/
/*  Include Files                                                                                 */
/*================================================================================================*/


#include "json.hpp"

using Json = nlohmann::json;




/*================================================================================================*/
/*  Class                                                                                         */
/*================================================================================================*/


struct SExpression
{
};



template< typename DataType >
class ISerializable
{
protected:
    ISerializable( void )
    {
    }
    virtual ~ISerializable( void )
    {
    }

public:
    virtual void Read( const DataType& /*_InData*/ )
    {
    }
    virtual void Write( DataType& /*_OutData*/ ) const
    {
    }
};


class IMultiSerializable: public ISerializable< SExpression >, public ISerializable< Json >
{
protected:
    IMultiSerializable( void )
    {
    }
    virtual ~IMultiSerializable( void )
    {
    }
};



class IDataObject
{
public:
    enum EType
    {
        Type_Invalid = -1, 
        Type_SExpression = 0, 
        Type_Json, 
    };

protected:
    IDataObject( EType _Type = EType::Type_Invalid ) : m_Type( _Type )
    {
    }
    virtual ~IDataObject( void )
    {
    }
    
public:

    EType GetType( void ) const
    {
        return m_Type;
    }

    template< typename DataType >
    bool IsSame( void ) const
    {
        return ( TypeTraits< DataType >::Value == m_Type );
    }
    

    template< typename SerializableVisitor >
    void Encode( const SerializableVisitor& _Visitor );

    template< typename SerializableVisitor >
    void Decode( SerializableVisitor& _Visitor ) const;

    virtual void SetData( const uint8_t* _Address ) = 0;
    virtual std::string ToString( void ) const = 0;
    virtual void Clear( void ) = 0;
    
    
    template< typename DataType > struct TypeTraits { enum { Value = EType::Type_Invalid, }; };
    template<> struct TypeTraits< SExpression > { enum { Value = EType::Type_SExpression, }; };
    template<> struct TypeTraits< Json > { enum { Value = EType::Type_Json, }; };

    // template< EType Value > struct Traits {  };
    // template<> struct Traits< EType::Type_SExpression > { using Type = SExpression; };
    // template<> struct Traits< EType::Type_Json > { using Type = Json; };

protected:
    EType           m_Type;
};


template< typename DataType >
class DataObject : public IDataObject
{
public:
    DataObject( void ) : IDataObject( static_cast< EType >( IDataObject::TypeTraits< DataType >::Value ) )
    {
    }
    virtual ~DataObject( void )
    {
    }
    

    // Type&    GetData( void )
    // {
    //     return m_Data;
    // }
    
    // const Type&    GetData( void ) const
    // {
    //     return m_Data;
    // }

    void Encode( const ISerializable< DataType >& _Visitor )
    {
        _Visitor.Write( m_Data );
    }

    void Decode( ISerializable< DataType >& _Visitor ) const
    {
        _Visitor.Read( m_Data );
    }

    virtual void SetData( const uint8_t* ) override
    {
    }
    virtual std::string ToString( void ) const override
    {
        return std::string();
    }
    virtual void Clear( void ) override
    {
    }


protected:
    DataType       m_Data;
};


template<>
void DataObject< SExpression >::SetData( const uint8_t* )
{
}

template<>
void DataObject< Json >::SetData( const uint8_t* _Address )
{
    m_Data = Json::parse( _Address );
}


template<>
std::string DataObject< SExpression >::ToString( void ) const
{
    return std::string();
}

template<>
std::string DataObject< Json >::ToString( void ) const
{
    if ( m_Data.empty() )
    {
        return std::string();
    }

    return m_Data.dump();
}


template<>
void DataObject< SExpression >::Clear( void )
{
}

template<>
void DataObject< Json >::Clear( void )
{
    m_Data.clear();
}





/*================================================================================================*/
/*  Class Inline Method                                                                           */
/*================================================================================================*/


template< typename SerializableVisitor >
void IDataObject::Encode( const SerializableVisitor& _Visitor )
{
    switch ( m_Type )
    {
        case EType::Type_SExpression:
            {
                auto    data_object = reinterpret_cast< DataObject< SExpression >* >( this );

                data_object->Encode( _Visitor );
            }
            break;
        case EType::Type_Json:
            {
                auto    data_object = reinterpret_cast< DataObject< Json >* >( this );

                data_object->Encode( _Visitor );
            }
            break;
        default:
            break;
    }
}


template< typename SerializableVisitor >
void IDataObject::Decode( SerializableVisitor& _Visitor ) const
{
    switch ( m_Type )
    {
        case EType::Type_SExpression:
            {
                auto    data_object = reinterpret_cast< const DataObject< SExpression >* >( this );

                data_object->Decode( _Visitor );
            }
            break;
        case EType::Type_Json:
            {
                auto    data_object = reinterpret_cast< const DataObject< Json >* >( this );

                data_object->Decode( _Visitor );
            }
            break;
        default:
            break;
    }
}





#endif
/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
