//
// Created by hjzh on 18-2-13.
//

#include "USBWebCamStreamDecodeThread.h"

void USBWebCamStreamDecodeThread::run() {

}

bool USBWebCamStreamDecodeThread::Init() {
  bool res = true;

  /// Register all formats and codecs
  av_register_all();

  if (RES_VID_OK != Connect()) {
    return res = false;
  }

  return false;
}

VID_ERR USBWebCamStreamDecodeThread::Connect() {
  VID_ERR res = RES_VID_OK;

  do {
    /// Open USB Web video stream, and allocate format context
    if (avformat_open_input(&m_pFormatCtx, m_config.m_szVideoStreamAddress.c_str(), NULL, NULL) < 0) {
      m_logger.error(Poco::format("USBWebCamStreamDecodeThread::Connect "
                                      "Can't open the video stream with this address: %s",
                                  m_config.m_szVideoStreamAddress));
      res = RES_VID_ERR_OPEN_INPUT;
      break;
    }

    /// Try to find a stream
    if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0) {
      m_logger.error(Poco::format("USBWebCamStreamDecodeThread::Connect "
                                      "Can't find the stream info: %s", m_config.m_szVideoStreamAddress));
      res = RES_VID_ERR_FIND_STREAM_INFO;
      break;
    }

    /// Try to find the video stream
    m_nVideoStreamIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &m_pDecoder, 0);
    if (m_nVideoStreamIndex < 0) {
      m_logger.error(Poco::format("USBWebCamStreamDecodeThread::Connect "
                                      "Can't find the video stream: %s", m_config.m_szVideoStreamAddress));
      res = RES_VID_ERR_FIND_VIDEO_STREAM;
      break;
    }

    m_pDecoderCtx = avcodec_alloc_context3(m_pDecoder);
    if (!m_pDecoderCtx) {
      m_logger.error(Poco::format("USBWebCamStreamDecodeThread::Connect Failed to allocate the %s codec context",
                                  av_get_media_type_string(AVMEDIA_TYPE_VIDEO)));
      res = RES_VID_ERR_ALLOC_CODEC_CTX;
      break;
    }

    m_pVideo = m_pFormatCtx->streams[m_nVideoStreamIndex];

    /// Copy codec parameters from input stream to output codec context
    if (avcodec_parameters_to_context(m_pDecoderCtx, m_pVideo->codecpar) < 0) {
      m_logger.error(Poco::format("USBWebCamStreamDecodeThread::Connect failed to copy %s codec "
                                      "parameters to decoder context", av_get_media_type_string(AVMEDIA_TYPE_VIDEO)));
      res = RES_VID_ERR_COPY_CODEC_PAR;
      break;
    }

  } while (false);


}