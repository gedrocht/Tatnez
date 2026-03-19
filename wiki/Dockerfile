FROM ruby:3.3-bookworm

RUN apt-get update \
  && apt-get install --yes --no-install-recommends build-essential git libicu-dev pkg-config \
  && rm -rf /var/lib/apt/lists/*

RUN gem install --no-document gollum

WORKDIR /workspace

COPY scripts/entrypoint.sh /entrypoint.sh

RUN chmod +x /entrypoint.sh

EXPOSE 4567

ENTRYPOINT ["/entrypoint.sh"]
