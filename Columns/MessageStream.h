#pragma once

#include <variant>
#include <unordered_map>
#include <type_traits>

namespace geng
{
	template<typename TMsg, typename TArchive>
	struct Encoder;

	template<typename TMsg, typename TArchive>
	struct Decoder;

	template<typename TMsg>
	struct MessageTraits
	{
		static auto msgType = TMsg::GetType();
	};

	// Stores one read message 
	template<typename TMsgTypeField, typename ... Msgs>
	class MessageInputOutput
	{
	private:
		using TCreatorProc = bool(MessageInputOutput::*)();
		using TInstantiatorMap = std::unordered_map<TMsgTypeField, TCreatorProc>;

		template<typename TMsg>
		void FillInstantiatorMap(TInstantiatorMap& instMap)
		{
			auto msgType = MessageTraits<TMsg>::msgType;
			if (instMap.count(msgType) == 0)
			{
				instMap.emplace(msgType,
					&MessageInputOutput::CreateMessage<TMsg>);
			}
		}

		template<typename TMsg, typename ... TRest>
		void FillInstantiatorMap(TInstantiatorMap& instMap)
		{
			auto msgType = MessageTraits<TMsg>::msgType;
			if (instMap.count(msgType) == 0)
			{
				instMap.emplace(msgType, 
					&MessageInputOutput::CreateMessage<TMsg>);
			}

			FillInstantiatorMap<TRest...>(instMap);
		}

		template<typename ... Msg>
		static TInstantiatorMap
			GenInstantiatorMap()
		{
			 procMap;
			FillInstantiatorMap<Msg...>(procMap);
		}

		static TInstantiatorMap sm_instantiatorMap
			= GenInstantiatorMap<Msgs...>();

		static bool
			GetInstantiatorForType(TMsgTypeField msgType,
				TCreatorProc& rProc)
		{
			auto itMsg = sm_instantiatorMap.find(msgType);
			if (itMsg != sm_instantiatorMap.end())
			{
				rProc = itMsg->second;
				return true;
			}

			return false;
		}
	public:

		template<typename TMsg>
		bool CreateMessage()
		{
			m_msgVariant<TMsg>.emplace();
			return true;
		}

		template<typename F>
		bool VisitMessage(F&& visitor)
		{
			std::visit(visitor, m_msgVariant);
		}

		template<typename TMsg, typename TArchive>
		bool EmitMessage(TArchive& targetStream)
		{
			// Encode the message into the target archive
			Encoder<TMsg, TArchive>  msgEncoder;
			bool encodeResult{ false };

			auto encodeVisitor = [&msgEncoder,
				&encodeResult,
				&targetStream](auto& msgToEncode)
			{
				msgEncoder.SetMsgType(MessageTraits<TMsg>::msgType);
				encodeResult = msgEncoder.Encode(targetStream, msgToEncode);
			};

			std::visit(encodeVisitor, m_msgVariant);
		}

		template<typename TArchive>
		bool ConsumeMessage(TArchive& sourceStream)
		{
			Decoder<TMsgTypeField,TArchive> typeDecoder;
			
			TMsgTypeField msgType;
			bool consumedType = typeDecoder.Decode(sourceStream, msgType);
			if (!consumedType)
			{
				return false;
			}

			TCreatorProc creatorProc;
			// Create the message of this type, and then decode using a visitor
			if (!GetInstantiatorForType(msgType, creatorProc))
			{
				return false;
			}

			if (!this->*creatorProc())
			{
				return false;
			}

			bool decodeResult{ false };
			auto decodeVisitor = [&decodeResult, &sourceStream]
			(auto& msgToDecode)
			{
				Decoder < std::remove_reference_t<decltype(msgToDecode),
					TArchive> msgDecoder;

				decodeResult = msgDecoder.Decode(sourceStream, msgToDecode);
			};

			std::visit(decodeVisitor, m_msgVariant);
		}

	private:
		std::variant<Msgs...>  m_msgVariant;
	};


}