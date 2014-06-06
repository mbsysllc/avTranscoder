#include <AvTranscoder/InputFile.hpp>
#include <AvTranscoder/DatasStructures/AudioDesc.hpp>

#include <AvTranscoder/Options/OptionInt.hpp>
#include <AvTranscoder/Options/OptionDouble.hpp>
#include <AvTranscoder/Options/OptionString.hpp>
#include <AvTranscoder/Options/OptionBoolean.hpp>
#include <AvTranscoder/Options/Option2D.hpp>
#include <AvTranscoder/Options/OptionGroup.hpp>
#include <AvTranscoder/Options/OptionChoice.hpp>
#include <AvTranscoder/Option.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

int optionChecker( const std::string& inputfilename )
{
	avtranscoder::InputFile inputFile( inputfilename );
	inputFile.analyse();
	
	avtranscoder::AudioDesc audioDesc( inputFile.getStream( 0 ).getAudioDesc() );
	
	std::vector<avtranscoder::Option*> options;
	std::vector<avtranscoder::OptionChoice*> optionsChoice; // need to access easely to the OptionChoice to add the list of value for each of them
	try
	{
		const AVOption* avOption = NULL;
		
		while( ( avOption = av_opt_next( audioDesc.getCodecContext(), avOption ) ) != NULL )
		{
			avtranscoder::Option* opt = NULL;
			
			if( !avOption || ! avOption->name )
			{
				continue;
			}
			
			std::cout << "The option is " << avOption->name << " of type : " << avOption->type << std::endl;

			if( avOption->unit && avOption->type == AV_OPT_TYPE_FLAGS )
			{
				opt = new avtranscoder::OptionGroup( *avOption );
				options.push_back( opt );
				continue;
			}
			if( avOption->unit && avOption->type == AV_OPT_TYPE_INT )
			{
				opt = new avtranscoder::OptionChoice( *avOption );
				options.push_back( opt );
				optionsChoice.push_back( dynamic_cast<avtranscoder::OptionChoice*>( opt ) );
				continue;
			}

			switch( avOption->type )
			{
				case AV_OPT_TYPE_FLAGS:
				{
					opt = new avtranscoder::OptionBoolean( *avOption );
					break;
				}
				case AV_OPT_TYPE_INT:
				case AV_OPT_TYPE_INT64:
				{
					opt = new avtranscoder::OptionInt( *avOption );
					break;
				}
				case AV_OPT_TYPE_DOUBLE:
				case AV_OPT_TYPE_FLOAT:
				{
					opt = new avtranscoder::OptionDouble( *avOption );
					break;
				}
				case AV_OPT_TYPE_STRING:
				{
					opt = new avtranscoder::OptionString( *avOption );
					break;
				}
				case AV_OPT_TYPE_RATIONAL:
				{
					opt = new avtranscoder::Option2D( *avOption );
					break;
				}
				case AV_OPT_TYPE_BINARY:
				{
					//opt = new avtranscoder::OptionString( *avOption );
					break;
				}
				case AV_OPT_TYPE_CONST:
				{
					break;
				}
				default:
				{
					std::cout << "----- Unknowed type for " << avOption->name << "-----" << std::endl;
					break;
				}
			}
			if( opt )
				options.push_back( opt );
		}
		
		// adding enum values and set default value for enums
		avOption = NULL;
		while( (avOption = av_opt_next( audioDesc.getCodecContext(), avOption)  ) != NULL )
		{
			if( !avOption ||
				!avOption->name ||
				( avOption->unit && avOption->type == AV_OPT_TYPE_FLAGS ) ||
				( avOption->unit && avOption->type == AV_OPT_TYPE_INT ) )
			{
				continue;
			}

			switch( avOption->type )
			{
				case AV_OPT_TYPE_CONST:
				{
					for( size_t i = 0; i < optionsChoice.size(); i++ )
					{
						if( avOption->unit == optionsChoice.at( i )->getUnit() )
						{
							if( avOption->help )
								optionsChoice.at( i )->appendOption( avOption->name, avOption->help );
							else
								optionsChoice.at( i )->appendOption( avOption->name );
							
							double valueDouble;
							if( avOption->default_val.dbl == optionsChoice.at( i )->getDefaultValue( valueDouble ) )
								optionsChoice.at( i )->setDefaultChoiceIndex( optionsChoice.at( i )->getNbChoices() - 1 );
						}
					}
				}
				default:
				{
					break;
				}
			}
		}
	}
	
	catch( std::exception &e )
	{
		for( auto it = options.begin(); it != options.end(); ++it)
		{
			delete *it;
		} 
		options.clear();
		
		throw e;
	}
	
	// display Options
	for( auto option : options )
	{
		std::cout << std::left;
		std::cout << "****************************" << std::endl;
		std::cout << std::setw(30) << option->getName();
		std::cout << ": " << option->getHelp() << std::endl;
		std::cout << "Type: " << option->getType() << std::endl;
		
		// get default value
		int valueInt;
		double valueDouble;
		std::string valueStr;
		bool valueBool;
		avtranscoder::Value2D value2D;
		
		if( option->getType() == "OptionInt" )
		{
			std::cout << "DefaultValue: " << option->getDefaultValue( valueInt ) << std::endl;
		}
		else if( option->getType() == "OptionBoolean" )
		{
			std::cout << "DefaultValue: " << option->getDefaultValue( valueBool ) << std::endl;
		}
		else if( option->getType() == "OptionDouble" )
		{
			std::cout << "DefaultValue: " << option->getDefaultValue( valueDouble ) << std::endl;
		}
		else if( option->getType() == "Option2D" )
		{
			option->getDefaultValue( value2D );
			std::cout << "DefaultValue: " << value2D.num << ", " << value2D.dem << std::endl;
		}
		else if( option->getType() == "OptionString" )
		{
			std::cout << "DefaultValue: " << option->getDefaultValue( valueStr ) << std::endl;
		}
		else if( option->getType() == "OptionChoice" )
		{
			std::cout << "DefaultValue: " << option->getDefaultValue( valueDouble ) << std::endl;
			
			avtranscoder::OptionChoice* choice = dynamic_cast<avtranscoder::OptionChoice*>( option );
			std::cout << "Nb choices: " << choice->getNbChoices() << std::endl;
			std::cout << "Default choice index: " << choice->getDefaultChoiceIndex() << std::endl;
			for(size_t i = 0; i < choice->getChoices().size(); ++i )
				std::cout << "Choice " << i << ": " << choice->getChoice( i ).first << " // " << choice->getChoice( i ).second << std::endl;
		}
		else if( option->getType() == "OptionChoice" )
		{
			std::cout << "DefaultValue: " << option->getDefaultValue( valueInt ) << std::endl;
		}
	}
}

int main( int argc, char** argv )
{
	if( argc <= 1 )
	{
		std::cout << "audiorewrapper require a media filename" << std::endl;
		std::cout << "example: audioWrap file.ext" << std::endl;
		return( -1 );
	}

	std::cout << "start ..." << std::endl;

	try
	{
		optionChecker( argv[1] );
	}
	catch( std::exception &e )
	{
		std::cout << "[ERROR] " << e.what() << std::endl;
	}
	
	std::cout << "end ..." << std::endl;
}