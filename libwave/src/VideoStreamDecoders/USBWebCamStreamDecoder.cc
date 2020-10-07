//
// Created by hejia on 18-2-13.
//

#include "VideoStreamDecoders/USBWebCamStreamDecoder.h"

void USBWebCamStreamDecoder::run() {
  AVFrame *pYUVFrame = av_frame_alloc();
  AVFrame *pBGRFrame = av_frame_alloc();

  uint8_t *buffer;
  int buffer_size;
  AVPixelFormat pix_fmt = AV_PIX_FMT_BGR24;
  int align = 32;
  buffer_size = av_image_get_buffer_size(pix_fmt, m_pDecoderCtx->width, m_pDecoderCtx->height, align);

  buffer = (unsigned char *) (av_malloc(buffer_size * sizeof(uint8_t)));
  av_image_fill_arrays(pBGRFrame->data, pBGRFrame->linesize, buffer, AV_PIX_FMT_BGR24, m_pDecoderCtx->width,
                       m_pDecoderCtx->height, align);

  AVPacket packet;

  /// Initialize packet, set data to NULL, let the demuxer fill it
  av_init_packet(&packet);
  packet.data = NULL;
  packet.size = 0;

  Poco::Int64 nFrameNum = 0;
  int break_status = 0;
  while (!m_stop) {
    if (av_read_frame(m_pFormatCtx, &packet) != 0) {
      m_bConnected = false;
      break_status = 1;
      break;
    }

    /// Begin to decode packet
    if (packet.stream_index == m_nVideoStreamIndex) {
      /// Decode video frame
      int got_picture = 0;
      int res = avcodec_decode_video2(m_pDecoderCtx, pYUVFrame, &got_picture, &packet);
      if (res >= 0 && got_picture) {
        ++nFrameNum;
        if (m_stop) {
          break_status = 2;
          break;
        }

        SharedIplImage pImg;
        if (m_config.m_ifResize) {
          pImg = cvCreateImage(cvSize(m_config.m_resizedWidth, m_config.m_resizedHeight), 8, 3);
        } else {
          pImg = cvCreateImage(cvSize(m_pDecoderCtx->width, m_pDecoderCtx->height), 8, 3);
        }

        struct SwsContext *img_convert_ctx;
        img_convert_ctx = sws_getContext(m_pDecoderCtx->width, m_pDecoderCtx->height, m_pDecoderCtx->pix_fmt,
                                         m_pDecoderCtx->width, m_pDecoderCtx->height, AV_PIX_FMT_BGR24,
                                         SWS_BICUBIC, NULL, NULL, NULL);
        sws_scale(img_convert_ctx, pYUVFrame->data, pYUVFrame->linesize, 0,
                  m_pDecoderCtx->height, pBGRFrame->data, pBGRFrame->linesize);
        cv::Mat ori_img(m_pDecoderCtx->height, m_pDecoderCtx->width, CV_8UC3, pBGRFrame->data[0]);

        ImageFrame data(ori_img);
        data.frameIdx = nFrameNum;
        m_frameCallback(data);
      }
    }
    av_free_packet(&packet);
  }
  av_read_pause(m_pFormatCtx);
  av_frame_free(&pYUVFrame);
  av_frame_free(&pBGRFrame);
  av_free(buffer);

  avcodec_free_context(&m_pDecoderCtx);
  avformat_close_input(&m_pFormatCtx);
}

bool USBWebCamStreamDecoder::Init() {
  bool res = true;

  /// Register all formats and codecs
  av_register_all();
  avcodec_register_all();
  avformat_network_init();
  avfilter_register_all();
  avdevice_register_all();

  if (RES_VID_OK != Connect()) {
    res = false;
  }

  return res;
}

VID_ERR USBWebCamStreamDecoder::Connect() {
  VID_ERR res = RES_VID_OK;

  do {
    /// Open USB Web video stream, and allocate format context
    if (avformat_open_input(&m_pFormatCtx, m_config.m_szVideoStreamAddress.c_str(), NULL, NULL) < 0) {
      std::stringstream message;
      message << "USBWebCamStreamDecoder::Connect "
                 "Can't open the video stream with this address: " << m_config.m_szVideoStreamAddress;
      throw std::runtime_error(message.str());
    }

    /// Try to find a stream
    if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0) {
      std::stringstream message;
      message << "USBWebCamStreamDecoder::Connect "
                 "Can't find the stream info: " << m_config.m_szVideoStreamAddress;
      throw std::runtime_error(message.str());
    }

    /// Try to find the video stream
    m_nVideoStreamIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &m_pDecoder, 0);
    if (m_nVideoStreamIndex < 0) {
      std::stringstream message;
      message << "USBWebCamStreamDecoder::Connect "
                 "Can't find the video stream: " << m_config.m_szVideoStreamAddress;
      throw std::runtime_error(message.str());
    }

    m_pDecoderCtx = avcodec_alloc_context3(m_pDecoder);
    if (!m_pDecoderCtx) {
      std::stringstream message;
      message << "USBWebCamStreamDecoder::Connect Failed to allocate the"
              << av_get_media_type_string(AVMEDIA_TYPE_VIDEO) << "codec context";
      throw std::runtime_error(message.str());
    }

    m_pVideo = m_pFormatCtx->streams[m_nVideoStreamIndex];

    /// Copy codec parameters from input stream to output codec context
    if (avcodec_parameters_to_context(m_pDecoderCtx, m_pVideo->codecpar) < 0) {
      std::stringstream message;
      message << "USBWebCamStreamDecoder::Connect failed to copy " << av_get_media_type_string(AVMEDIA_TYPE_VIDEO)
              << " codec parameters to decoder context";
      throw std::runtime_error(message.str());
    }

    /// Init the decoders
    if (avcodec_open2(m_pDecoderCtx, m_pDecoder, NULL) < 0) {
      std::stringstream message;
      message << "USBWebCamStreamDecoder::Connect Can't open decoder: " << m_pDecoder->name;
      throw std::runtime_error(message.str());
    }
  } while (false);
  /// If failed in connection, release the resources
  if (res != RES_VID_OK) {
    avformat_close_input(&m_pFormatCtx);
    if (m_pDecoderCtx) {
      avcodec_free_context(&m_pDecoderCtx);
    }
  } else {
    m_bConnected = true;
  }
  return res;
}