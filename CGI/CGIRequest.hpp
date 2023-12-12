#ifndef CGIREQUEST_H
# define  CGIREQUEST_H

class  CGIRequest {
public:
    CGIRequest();
    CGIRequest(const CGIRequest &obj);
    ~CGIRequest();
    CGIRequest &operator=(const CGIRequest &obj);
};

#endif
