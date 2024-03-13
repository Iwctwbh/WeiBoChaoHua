#include <QCoreApplication>
#include <cpr/cpr.h>
#include <fmt/format.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>

#undef min
#undef max

#include <xlnt/xlnt.hpp>

#pragma execution_character_set("utf-8")

struct Blog
{
	QString id;
	QString text;
	QString forward;
	QString comment;
	QString like;
	QString url;
	QString username;
	QString userid;
	QString time;
};

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	indicators::BlockProgressBar bar{
		indicators::option::BarWidth{50},
		indicators::option::PrefixText{"Downloading:"},
		indicators::option::ShowRemainingTime{true},
		indicators::option::ShowElapsedTime{true},
		indicators::option::ForegroundColor{indicators::Color::white},
		indicators::option::FontStyles{
			std::vector<indicators::FontStyle>{indicators::FontStyle::bold}
		},
		indicators::option::MaxProgress{50000}
	};

	indicators::show_console_cursor(false);

	QString url_base{};
	QString cookie{};
	QString count{};

	QFile file_config("config.json");
	QJsonDocument json_config{};
	if (file_config.open(QIODevice::ReadOnly))
	{
		json_config = QJsonDocument::fromJson(file_config.readAll());
		url_base = json_config.object().value("Url").toVariant().toString();
		cookie = json_config.object().value("Cookie").toVariant().toString();
		count = json_config.object().value("Count").toVariant().toString();
	}
	else
	{
		qDebug() << "failed to open config file";
		system("pause");
		exit(0);
	}

	auto response_init = cpr::Get(cpr::Url{url_base.toUtf8().constData()},
	                              cpr::Header{
		                              {"cookie", cookie.toUtf8().constData()}
	                              },
	                              cpr::Body{R"([{}])"},
	                              cpr::Timeout(50000),
	                              cpr::ConnectTimeout(50000)
	);
	QString response_text = QString::fromLocal8Bit(response_init.text);
	const QJsonDocument json{QJsonDocument::fromJson(response_text.toLocal8Bit())};

	QString since_id = json.object().value("data").toObject().value("pageInfo").toObject().value("since_id").toVariant().toString();
	QString total_blog = json.object().value("data").toObject().value("pageInfo").toObject().value("total").toVariant().toString();
	QString page_title = json.object().value("data").toObject().value("pageInfo").toObject().value("page_title").toVariant().toString();

	QMap<QString, Blog> blog_map{};
	bar.set_option(indicators::option::MaxProgress{total_blog.toUInt()});
	std::ranges::for_each(json.object().value("data").toObject().value("cards").toArray(),
	                      [&blog_map, &bar, &total_blog](const auto& card)
	                      {
		                      if (card.toObject().value("card_group").toArray().at(0).toObject().value("card_type").toVariant().toString() == "9")
		                      {
			                      QJsonArray card_group{card.toObject().value("card_group").toArray()};
			                      for (const auto& c : card_group)
			                      {
				                      QJsonObject mblog{c.toObject().value("mblog").toObject()};
				                      Blog blog{};
				                      blog.id = mblog.value("id").toVariant().toString();
				                      blog.text = mblog.value("text").toVariant().toString();
				                      blog.forward = mblog.value("reposts_count").toVariant().toString();
				                      blog.like = mblog.value("attitudes_count").toVariant().toString();
				                      blog.comment = mblog.value("comments_count").toVariant().toString();
				                      blog.username = mblog.value("user").toObject().value("screen_name").toVariant().toString();
				                      blog.userid = mblog.value("user").toObject().value("id").toVariant().toString();
				                      blog.url = QString("https://weibo.com/%1/%2").arg(blog.userid, blog.id);
				                      blog.time = mblog.value("created_at").toVariant().toString();
				                      blog_map.insert(blog.id, blog);

				                      bar.set_option(indicators::option::PostfixText{
					                      std::to_string(bar.current()) + "/" + total_blog.toUtf8().constData()
				                      });
				                      bar.tick();
			                      }
		                      }
	                      });

	const QString url{url_base + "_-_feed&since_id="};

	for (int i = 0; i < count.toInt(); ++i)
	{
		auto response_since = cpr::Get(cpr::Url{
			                               QString("%1%2").arg(url, since_id).toUtf8().constData()
		                               },
		                               cpr::Header{
			                               {"cookie", cookie.toUtf8().constData()}
		                               },
		                               cpr::Timeout(50000),
		                               cpr::ConnectTimeout(50000)
		);

		response_text = QString::fromUtf8(response_since.text);
		const QJsonDocument json_since{QJsonDocument::fromJson(response_text.toUtf8())};
		QJsonArray card_group{
			json_since.object().value("data").toObject().value("cards").toArray().at(0).toObject().value("card_group").toArray()
		};

		for (const auto& card : card_group)
		{
			QJsonObject mblog{card.toObject().value("mblog").toObject()};
			Blog blog{};
			blog.id = mblog.value("id").toVariant().toString();
			blog.text = mblog.value("text").toVariant().toString();
			blog.forward = mblog.value("reposts_count").toVariant().toString();
			blog.like = mblog.value("attitudes_count").toVariant().toString();
			blog.comment = mblog.value("comments_count").toVariant().toString();
			blog.username = mblog.value("user").toObject().value("screen_name").toVariant().toString();
			blog.userid = mblog.value("user").toObject().value("id").toVariant().toString();
			blog.url = QString("https://weibo.com/%1/%2").arg(blog.userid, blog.id);
			blog.time = mblog.value("created_at").toVariant().toString();
			blog_map.insert(blog.id, blog);

			bar.set_option(indicators::option::PostfixText{
				std::to_string(bar.current()) + "/" + total_blog.toUtf8().constData()
			});
			bar.tick();
		}

		since_id = json_since.object().value("data").toObject().value("pageInfo").toObject().value("since_id").toVariant().toString();

		if (since_id.isEmpty())
		{
			break;
		}
	}

	qDebug() << "\ntotal:" << blog_map.size() << '\n';

	xlnt::workbook wb;
	xlnt::worksheet ws = wb.active_sheet();

	int row = 1;

	ws.cell("A", row).value("text");
	ws.cell("B", row).value("forward");
	ws.cell("C", row).value("comment");
	ws.cell("D", row).value("like");
	ws.cell("E", row).value("username");
	ws.cell("F", row).value("time");
	ws.cell("G", row).value("url");

	++row;

	foreach(auto blog, blog_map)
	{
		ws.cell("A", row).value(blog.text.toUtf8());
		ws.cell("B", row).value(blog.forward.toUtf8());
		ws.cell("C", row).value(blog.comment.toUtf8());
		ws.cell("D", row).value(blog.like.toUtf8());
		ws.cell("E", row).value(blog.username.toUtf8());
		ws.cell("F", row).value(blog.time.toUtf8());
		ws.cell("G", row).value(blog.url.toUtf8());
		++row;
	}

	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_date = current_date_time.toString("yyyyMMdd");

	wb.save((page_title + "_" + current_date + ".xlsx").toUtf8().constData());

	system("pause");
	exit(0);

	return QCoreApplication::exec();
}
